
# Battle link communications

## Globals and state variables

| Name | Description |
| :- | :- |
| `gReceivedRemoteLinkPlayers` |
| `gBlockRecvBuffer[]` | Raw buffers for inbound link communications from each linked console. |
| `gLinkBattleSendBuffer` | Storage for messages that will be sent to other linked consoles. Each message is written after the previous (with 4-byte alignment) until the end of the buffer is reached, at which point we jump back to the start. |
| `gLinkBattleRecvBuffer` | Storage for messages that have been received from other linked consoles. Works essentially the same as the "send" buffer. |
| `gBattleBufferA[]` | Storage for the current message sent from the battle engine to a battle controller. Holds the current engine-to-controller message for each battler. |
| `gBattleBufferB[]` | Storage for the current message sent from a battle controller to the battle engine. Holds the current controller-to-engine message for each battler. |
| `gBattleControllerExecFlags` |

## Data formats

### Link message

```c
enum {
   BATTLELINKMSGTYPE_ENGINE_TO_CONTROLLER     = 0, // gBattleBufferA
   BATTLELINKMSGTYPE_CONTROLLER_TO_ENGINE     = 1, // gBattleBufferB
   BATTLELINKMSGTYPE_CONTROLLER_BECOMING_IDLE = 2, // data[0] is the sender's multiplayer ID
};

struct {
   u8        message_type; // BATTLELINKMSGTYPE_
   BattlerID activeBattler;
   BattlerID attacker;
   BattlerID target;
   u16       size; // little-endian
   u8        absentBattlerFlags;
   BattlerID effectBattler;
   u8        data[];
};
```

## "Send" task

Task fields for `Task_HandleSendLinkBuffersData`:

* **`data[10]`:** Countdown timer, for task states that advance after a delay.

* **`data[11]`:** State enum for the task.

* **`data[12]`:** Some kind of offset into `gLinkBattleSendBuffer`. At the start of `PrepareBufferDataTransferLink`, if there's insufficient space, then `data[12]` is set to `data[14]` and `data[14]` is set to 0.

* **`data[13]`:** Countdown timer to advance past states 3 and 5? (State 5 is never used?)

* **`data[14]`:** Some kind of offset into `gLinkBattleSendBuffer`. The `PrepareBufferDataTransferLink` function writes battle link data to that offset, and then advances `data[14]` forward, endeavoring to keep it 4-byte-aligned.

* **`data[15]`:** Some kind of offset into `gLinkBattleSendBuffer`.

Current theory:

* `gLinkBattleSendBuffer` is a single buffer, but it's handled akin to a ring buffer. 
* Send Task State 3: `data[15]` is the end-offset of the last datablock sent over the link, and therefore also the start-offset of the next datablock to be sent. It and `data[14]` will only differ when `PrepareBufferDataTransferLink` builds that next datablock and sets `data[14]` to its end-offset.
  * If `data[15] > data[14] && data[15] == data[12]`, then we have exhausted the buffer and need to wrap back around to its start, so both values are reset to zero. Notably, this condition will occur if `PrepareBufferDataTransferLink` detects that it's too close to the end of the buffer and sets `data[14]` to 0 and `data[12]` to the value `data[14]` previously had (which, implicitly, would be `data[15]`).
* Send Task State 4: Once `IsLinkTaskFinished()` returns true, the "next" datablock has finished sending. As such, `data[15]` is set to the end-offset of that datablock (the initial conditions for Send Task State 3).

Ergo:

| Data field | Name |
| :- | :- |
| `data[10]` | `tInitialDelayTimer` |
| `data[11]` | `tState` |
| `data[12]` | `tBufferWrappingAroundFrom` / `tToBeSentBlock_WrapFrom` |
| `data[13]` | `tBlockSendDelayTimer` |
| `data[14]` | `tToBeSentBlock_End` |
| `data[15]` | `tLastSentBlock_End` / `tToBeSentBlock_Start` |

## "Receive" task

Task fields for `Task_HandleCopyReceivedLinkBuffersData`:

* **`data[12]`:** Same as the "send" task, but for receipt of blocks?
* **`data[13]`:** Unused?
* **`data[14]`:** Same as the "send" task, but for receipt of blocks?
* **`data[15]`:** Same as the "send" task, but for receipt of blocks?