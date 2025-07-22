define connect_to_mgba
   target remote :2345
   symbol-file ./pokeemerald_modern.elf
   rwatch *0
end