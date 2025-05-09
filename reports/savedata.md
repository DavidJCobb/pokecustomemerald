# Savedata format

<!-- This file is auto-generated by a post-build script. Changes made to it will be lost.. -->

This project uses an automatically-generated savedata format wherein savedata is "bitpacked" to optimize for storage space. The vanilla savedata format consumes **96.23%** of the available space, leaving 2096 bytes to spare. The bitpacked format consumes **86.53%** of that space, while wasting **0.46%** of the remaining space due to technical limitations, thereby leaving 7230 bytes to spare.

## Stats

### Stats per sector

| Sector | Unpacked bytes | Unpacked % | Packed bytes | Packed bits | Packed % |
| -: | -: | -: | -: | -: | -: |
|0|3960|99.80%|3415|27317|86.05%|
|1|3968|100.00%|3968|31743|100.00%|
|2|3968|100.00%|3968|31740|99.99%|
|3|3968|100.00%|3943|31544|99.37%|
|4|3848|96.98%|2041|16324|51.42%|
|5|3968|100.00%|3936|31486|99.19%|
|6|3968|100.00%|3936|31482|99.17%|
|7|3968|100.00%|3936|31482|99.17%|
|8|3968|100.00%|3936|31482|99.17%|
|9|3968|100.00%|3936|31482|99.17%|
|10|3968|100.00%|3936|31482|99.17%|
|11|3968|100.00%|3936|31482|99.17%|
|12|3968|100.00%|3187|25494|80.31%|
|13|2000|50.40%|0|0|0.00%|
|**Used**|53456|96.23%|48068|384540|86.53%|
|**Lost**|||255|2035|0.46%|
|**Free**|2096|3.77%|7230|396094|13.02%|

Some values can't be split across sectors when they're bitpacked. If these values can't fit at the end of a sector, then they must be pushed to the start of the next sector &mdash; leaving unused space at the end of the sector they were pushed past. This space is listed as "lost" in the table above.[^vanilla-never-loses-space]

[^vanilla-never-loses-space]: The vanilla game never produces "lost" space because it just uses `memcpy` to copy data directly between RAM and flash memory, blindly slicing values at sector boundaries. By contrast, our generated bitpacking code can only slice aggregates (e.g. arrays, structs, unions); it doesn't support slicing primitives (i.e. integers), strings, or opaque buffers. Some aggregates are transformed into other struct types (e.g. `BoxPokemon` to `SerializedBoxPokemon`), and not all of these transformations will be sliceable either.

Some top-level values are also deliberately forced to align with the start of a sector, rather than sharing a sector with any preceding value. (This is generally done to maintain compatibility with game code that attempts to load certain parts of the savegame one by one.) These values are not counted as creating "lost" space at the end of the previous sector.

### Bitpack value categories

Types or values can be annotated with category names. Category names have no effect on how values are packed; they are purely an informational tool for external tools which read the bitpack format XML (e.g. the tool which produced this report).

#### Size info

All sizes listed are totals. The **% used** column indicates how much of the total save file space is consumed by values of a given category. The **% size reduction** column is relative to the total unpacked size consumed by a given category.

| Categories | Count | Unpacked bytes | Packed bits | % used | % size reduction |
| :- | -: | -: | -: | -: | -: |
|**easy-chat-word**|1034|2068|16544|3.72%|0.00%|
|**game-id**|428|856|1712|0.39%|75.00%|
|**global-item-id**|1066|2132|17056|3.84%|0.00%|
|**language**|1642|1642|6568|1.48%|50.00%|
|**move-id**|2715|5430|24435|5.50%|43.75%|
|**player-gender**|474|902|474|0.11%|93.43%|
|**player-name**|1520|11708|85120|19.15%|9.12%|
|**player-name-mauville-old-man**|4|44|320|0.07%|9.09%|
|**pokemon-level**|659|659|4613|1.04%|12.50%|
|**pokemon-name**|596|6128|47680|10.73%|2.74%|
|**species-id**|1360|2720|14960|3.37%|31.25%|
|**trainer-id**|591|842|6640|1.49%|1.43%|

#### Counts by sector

| Category | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | Total |
| :- | -: | -: | -: | -: | -: | -: | -: | -: | -: | -: | -: | -: | -: | -: | -: |
|**easy-chat-word**|150|||869|15||||||||||1034|
|**game-id**||6||2||54|54|54|54|54|54|54|42||428|
|**global-item-id**|59|243|78|262|4|54|54|54|54|54|54|54|42||1066|
|**language**|71|6|14|1128|3|54|54|54|54|54|54|54|42||1642|
|**move-id**|168|24|312|531||216|216|216|216|216|216|216|168||2715|
|**player-gender**|1|6|14|33||54|54|54|54|54|54|54|42||474|
|**player-name**|81|6|14|976|23|54|54|54|54|54|54|54|42||1520|
|**player-name-mauville-old-man**||||4|||||||||||4|
|**pokemon-level**|28|12|78|121||54|54|54|54|54|54|54|42||659|
|**pokemon-name**|28|6||142||54|54|54|54|54|54|54|42||596|
|**species-id**|50|6|78|804|2|54|54|54|54|54|54|54|42||1360|
|**trainer-id**|383||56|118|34||||||||||591|

#### Counts by top-level value

| Category | gSaveBlock2Ptr | gCustomGameOptions | gSaveBlock1Ptr | gCustomGameSavestate | gPokemonStoragePtr | Total |
| :- | -: | -: | -: | -: | -: | -: |
|**easy-chat-word**|150||884|||1034|
|**game-id**|||8||420|428|
|**global-item-id**|59||587||420|1066|
|**language**|71||1151||420|1642|
|**move-id**|168||867||1680|2715|
|**player-gender**|1||53||420|474|
|**player-name**|81||1019||420|1520|
|**player-name-mauville-old-man**|||4|||4|
|**pokemon-level**|27|1|211||420|659|
|**pokemon-name**|28||148||420|596|
|**species-id**|47|3|890||420|1360|
|**trainer-id**|383||208|||591|

### Stats per struct/union type

**Note:** These listings make no effort to indicate when one struct commonly or only appears as a member of another struct. The listings only apply to types in a vacuum, not in context: a type that only appears within an "opaque buffer," for example, won't be bitpacked at all, but these listings won't distinguish between its theoretical space savings and the actual [lack of] space savings.

#### Size info

The **% used** column indicates how much of the total save file space is consumed by values of a given type.

<table>
<thead>
<tr>
<th></th><th colspan='3'>Sizes per instance</th><th></th><th colspan='3'>Total sizes</th></tr>
<tr>
<th style='text-align:left'>Typename</th><th>Unpacked bytes</th><th>Packed bits</th><th>Savings</th><th>Count</th><th>Unpacked bytes</th><th>Packed bits</th><th>% used</th></tr>
</thead>
<tbody style='text-align:right'>
<tr><th style='text-align:left'>Apprentice</th><td>68</td><td>432</td><td>20.59%</td><td>4</td><td>272</td><td>1728</td><td>0.39%</td></tr>
<tr><th style='text-align:left'>ApprenticeMon</th><td>12</td><td>63</td><td>34.38%</td><td>12</td><td>144</td><td>756</td><td>0.17%</td></tr>
<tr><th style='text-align:left'>ApprenticeQuestion</th><td>4</td><td>24</td><td>25.00%</td><td>9</td><td>36</td><td>216</td><td>0.05%</td></tr>
<tr><th style='text-align:left'>BattleDomeTrainer</th><td>4</td><td>16</td><td>50.00%</td><td>16</td><td>64</td><td>256</td><td>0.06%</td></tr>
<tr><th style='text-align:left'>BattleFrontier</th><td>2272</td><td>16110</td><td>11.37%</td><td>1</td><td>2272</td><td>16110</td><td>3.62%</td></tr>
<tr><th style='text-align:left'>BattleTowerEReaderTrainer</th><td>188</td><td>1370</td><td>8.91%</td><td>1</td><td>188</td><td>1370</td><td>0.31%</td></tr>
<tr><th style='text-align:left'>BattleTowerInterview</th><td>24</td><td>162</td><td>15.62%</td><td>1</td><td>24</td><td>162</td><td>0.04%</td></tr>
<tr><th style='text-align:left'>BattleTowerPokemon</th><td>44</td><td>310</td><td>11.93%</td><td>27</td><td>1188</td><td>8370</td><td>1.88%</td></tr>
<tr><th style='text-align:left'>Berry2</th><td>28</td><td>208</td><td>7.14%</td><td>1</td><td>28</td><td>208</td><td>0.05%</td></tr>
<tr><th style='text-align:left'>BerryCrush</th><td>16</td><td>128</td><td>0.00%</td><td>1</td><td>16</td><td>128</td><td>0.03%</td></tr>
<tr><th style='text-align:left'>BerryPickingResults</th><td>16</td><td>128</td><td>0.00%</td><td>1</td><td>16</td><td>128</td><td>0.03%</td></tr>
<tr><th style='text-align:left'>BerryTree</th><td>8</td><td>48</td><td>25.00%</td><td>128</td><td>1024</td><td>6144</td><td>1.38%</td></tr>
<tr><th style='text-align:left'>BoxPokemon</th><td>80</td><td>583</td><td>8.91%</td><td>428</td><td>34240</td><td>249524</td><td>56.15%</td></tr>
<tr><th style='text-align:left'>ContestWinner</th><td>32</td><td>222</td><td>13.28%</td><td>13</td><td>416</td><td>2886</td><td>0.65%</td></tr>
<tr><th style='text-align:left'>Coords16</th><td>4</td><td>32</td><td>0.00%</td><td>49</td><td>196</td><td>1568</td><td>0.35%</td></tr>
<tr><th style='text-align:left'>CustomGameOptions</th><td>76</td><td>304</td><td>50.00%</td><td>1</td><td>76</td><td>304</td><td>0.07%</td></tr>
<tr><th style='text-align:left'>CustomGameSavestate</th><td>0</td><td>0</td><td>-nan%</td><td>1</td><td>0</td><td>0</td><td>0.00%</td></tr>
<tr><th style='text-align:left'>DayCare</th><td>288</td><td>2076</td><td>9.90%</td><td>1</td><td>288</td><td>2076</td><td>0.47%</td></tr>
<tr><th style='text-align:left'>DaycareMail</th><td>56</td><td>403</td><td>10.04%</td><td>2</td><td>112</td><td>806</td><td>0.18%</td></tr>
<tr><th style='text-align:left'>DaycareMon</th><td>140</td><td>1018</td><td>9.11%</td><td>2</td><td>280</td><td>2036</td><td>0.46%</td></tr>
<tr><th style='text-align:left'>DewfordTrend</th><td>8</td><td>63</td><td>1.56%</td><td>5</td><td>40</td><td>315</td><td>0.07%</td></tr>
<tr><th style='text-align:left'>DomeMonData</th><td>16</td><td>92</td><td>28.12%</td><td>3</td><td>48</td><td>276</td><td>0.06%</td></tr>
<tr><th style='text-align:left'>EmeraldBattleTowerRecord</th><td>236</td><td>1678</td><td>11.12%</td><td>6</td><td>1416</td><td>10068</td><td>2.27%</td></tr>
<tr><th style='text-align:left'>EnigmaBerry</th><td>52</td><td>400</td><td>3.85%</td><td>1</td><td>52</td><td>400</td><td>0.09%</td></tr>
<tr><th style='text-align:left'>ExternalEventData</th><td>20</td><td>160</td><td>0.00%</td><td>1</td><td>20</td><td>160</td><td>0.04%</td></tr>
<tr><th style='text-align:left'>ExternalEventFlags</th><td>21</td><td>163</td><td>2.98%</td><td>1</td><td>21</td><td>163</td><td>0.04%</td></tr>
<tr><th style='text-align:left'>GabbyAndTyData</th><td>12</td><td>79</td><td>17.71%</td><td>1</td><td>12</td><td>79</td><td>0.02%</td></tr>
<tr><th style='text-align:left'>ItemSlot</th><td>4</td><td>32</td><td>0.00%</td><td>236</td><td>944</td><td>7552</td><td>1.70%</td></tr>
<tr><th style='text-align:left'>LilycoveLady</th><td>64</td><td>317</td><td>38.09%</td><td>1</td><td>64</td><td>317</td><td>0.07%</td></tr>
<tr><th style='text-align:left'>LilycoveLadyContest</th><td>16</td><td>96</td><td>25.00%</td><td>1</td><td>16</td><td>96</td><td>0.02%</td></tr>
<tr><th style='text-align:left'>LilycoveLadyFavor</th><td>20</td><td>125</td><td>21.88%</td><td>1</td><td>20</td><td>125</td><td>0.03%</td></tr>
<tr><th style='text-align:left'>LilycoveLadyQuiz</th><td>44</td><td>317</td><td>9.94%</td><td>1</td><td>44</td><td>317</td><td>0.07%</td></tr>
<tr><th style='text-align:left'>LinkBattleRecord</th><td>16</td><td>120</td><td>6.25%</td><td>5</td><td>80</td><td>600</td><td>0.14%</td></tr>
<tr><th style='text-align:left'>LinkBattleRecords</th><td>88</td><td>620</td><td>11.93%</td><td>1</td><td>88</td><td>620</td><td>0.14%</td></tr>
<tr><th style='text-align:left'>Mail</th><td>36</td><td>259</td><td>10.07%</td><td>18</td><td>648</td><td>4662</td><td>1.05%</td></tr>
<tr><th style='text-align:left'>MauvilleManBard</th><td>44</td><td>293</td><td>16.76%</td><td>1</td><td>44</td><td>293</td><td>0.07%</td></tr>
<tr><th style='text-align:left'>MauvilleManCommon</th><td>4</td><td>8</td><td>75.00%</td><td>1</td><td>4</td><td>8</td><td>0.00%</td></tr>
<tr><th style='text-align:left'>MauvilleManGiddy</th><td>36</td><td>252</td><td>12.50%</td><td>1</td><td>36</td><td>252</td><td>0.06%</td></tr>
<tr><th style='text-align:left'>MauvilleManHipster</th><td>4</td><td>13</td><td>59.38%</td><td>1</td><td>4</td><td>13</td><td>0.00%</td></tr>
<tr><th style='text-align:left'>MauvilleManStoryteller</th><td>60</td><td>409</td><td>14.79%</td><td>1</td><td>60</td><td>409</td><td>0.09%</td></tr>
<tr><th style='text-align:left'>MauvilleOldManTrader</th><td>56</td><td>384</td><td>14.29%</td><td>1</td><td>56</td><td>384</td><td>0.09%</td></tr>
<tr><th style='text-align:left'>MysteryGiftSave</th><td>876</td><td>6966</td><td>0.60%</td><td>1</td><td>876</td><td>6966</td><td>1.57%</td></tr>
<tr><th style='text-align:left'>ObjectEvent</th><td>36</td><td>276</td><td>4.17%</td><td>16</td><td>576</td><td>4416</td><td>0.99%</td></tr>
<tr><th style='text-align:left'>ObjectEventTemplate</th><td>24</td><td>160</td><td>16.67%</td><td>64</td><td>1536</td><td>10240</td><td>2.30%</td></tr>
<tr><th style='text-align:left'>OldMan</th><td>64</td><td>512</td><td>0.00%</td><td>1</td><td>64</td><td>512</td><td>0.12%</td></tr>
<tr><th style='text-align:left'>PlayersApprentice</th><td>44</td><td>261</td><td>25.85%</td><td>1</td><td>44</td><td>261</td><td>0.06%</td></tr>
<tr><th style='text-align:left'>Pokeblock</th><td>8</td><td>56</td><td>12.50%</td><td>40</td><td>320</td><td>2240</td><td>0.50%</td></tr>
<tr><th style='text-align:left'>Pokedex</th><td>120</td><td>960</td><td>0.00%</td><td>1</td><td>120</td><td>960</td><td>0.22%</td></tr>
<tr><th style='text-align:left'>Pokemon</th><td>100</td><td>742</td><td>7.25%</td><td>6</td><td>600</td><td>4452</td><td>1.00%</td></tr>
<tr><th style='text-align:left'>PokemonJumpRecords</th><td>16</td><td>80</td><td>37.50%</td><td>1</td><td>16</td><td>80</td><td>0.02%</td></tr>
<tr><th style='text-align:left'>PokemonStorage</th><td>33744</td><td>245872</td><td>8.92%</td><td>1</td><td>33744</td><td>245872</td><td>55.32%</td></tr>
<tr><th style='text-align:left'>PokemonSubstruct0</th><td>12</td><td>75</td><td>21.88%</td><td>428</td><td>5136</td><td>32100</td><td>7.22%</td></tr>
<tr><th style='text-align:left'>PokemonSubstruct1</th><td>12</td><td>68</td><td>29.17%</td><td>428</td><td>5136</td><td>29104</td><td>6.55%</td></tr>
<tr><th style='text-align:left'>PokemonSubstruct2</th><td>12</td><td>96</td><td>0.00%</td><td>428</td><td>5136</td><td>41088</td><td>9.25%</td></tr>
<tr><th style='text-align:left'>PokemonSubstruct3</th><td>12</td><td>96</td><td>0.00%</td><td>428</td><td>5136</td><td>41088</td><td>9.25%</td></tr>
<tr><th style='text-align:left'>PokeNews</th><td>4</td><td>32</td><td>0.00%</td><td>16</td><td>64</td><td>512</td><td>0.12%</td></tr>
<tr><th style='text-align:left'>PyramidBag</th><td>60</td><td>480</td><td>0.00%</td><td>1</td><td>60</td><td>480</td><td>0.11%</td></tr>
<tr><th style='text-align:left'>RamScript</th><td>1004</td><td>8024</td><td>0.10%</td><td>1</td><td>1004</td><td>8024</td><td>1.81%</td></tr>
<tr><th style='text-align:left'>RamScriptData</th><td>1000</td><td>7992</td><td>0.10%</td><td>1</td><td>1000</td><td>7992</td><td>1.80%</td></tr>
<tr><th style='text-align:left'>RankingHall1P</th><td>16</td><td>108</td><td>15.62%</td><td>54</td><td>864</td><td>5832</td><td>1.31%</td></tr>
<tr><th style='text-align:left'>RankingHall2P</th><td>28</td><td>196</td><td>12.50%</td><td>6</td><td>168</td><td>1176</td><td>0.26%</td></tr>
<tr><th style='text-align:left'>RecordMixingGift</th><td>16</td><td>128</td><td>0.00%</td><td>1</td><td>16</td><td>128</td><td>0.03%</td></tr>
<tr><th style='text-align:left'>RecordMixingGiftData</th><td>12</td><td>96</td><td>0.00%</td><td>1</td><td>12</td><td>96</td><td>0.02%</td></tr>
<tr><th style='text-align:left'>RentalMon</th><td>12</td><td>59</td><td>38.54%</td><td>6</td><td>72</td><td>354</td><td>0.08%</td></tr>
<tr><th style='text-align:left'>Roamer</th><td>28</td><td>147</td><td>34.38%</td><td>1</td><td>28</td><td>147</td><td>0.03%</td></tr>
<tr><th style='text-align:left'>SaveBlock1</th><td>15752</td><td>111351</td><td>11.64%</td><td>1</td><td>15752</td><td>111351</td><td>25.06%</td></tr>
<tr><th style='text-align:left'>SaveBlock2</th><td>3884</td><td>27013</td><td>13.06%</td><td>1</td><td>3884</td><td>27013</td><td>6.08%</td></tr>
<tr><th style='text-align:left'>SecretBase</th><td>160</td><td>1056</td><td>17.50%</td><td>20</td><td>3200</td><td>21120</td><td>4.75%</td></tr>
<tr><th style='text-align:left'>SecretBaseParty</th><td>108</td><td>660</td><td>23.61%</td><td>20</td><td>2160</td><td>13200</td><td>2.97%</td></tr>
<tr><th style='text-align:left'>SerializedBoxPokemonSubstructs</th><td>52</td><td>351</td><td>15.62%</td><td>428</td><td>22256</td><td>150228</td><td>33.80%</td></tr>
<tr><th style='text-align:left'>Time</th><td>8</td><td>36</td><td>43.75%</td><td>2</td><td>16</td><td>72</td><td>0.02%</td></tr>
<tr><th style='text-align:left'>TrainerHillSave</th><td>12</td><td>80</td><td>16.67%</td><td>1</td><td>12</td><td>80</td><td>0.02%</td></tr>
<tr><th style='text-align:left'>TrainerNameRecord</th><td>12</td><td>88</td><td>8.33%</td><td>20</td><td>240</td><td>1760</td><td>0.40%</td></tr>
<tr><th style='text-align:left'>TVShow</th><td>36</td><td>288</td><td>0.00%</td><td>25</td><td>900</td><td>7200</td><td>1.62%</td></tr>
<tr><th style='text-align:left'>WaldaPhrase</th><td>24</td><td>169</td><td>11.98%</td><td>1</td><td>24</td><td>169</td><td>0.04%</td></tr>
<tr><th style='text-align:left'>WarpData</th><td>8</td><td>56</td><td>12.50%</td><td>5</td><td>40</td><td>280</td><td>0.06%</td></tr>
<tr><th style='text-align:left'>WonderCard</th><td>332</td><td>2635</td><td>0.79%</td><td>1</td><td>332</td><td>2635</td><td>0.59%</td></tr>
<tr><th style='text-align:left'>WonderCardMetadata</th><td>36</td><td>283</td><td>1.74%</td><td>1</td><td>36</td><td>283</td><td>0.06%</td></tr>
<tr><th style='text-align:left'>WonderNews</th><td>444</td><td>3552</td><td>0.00%</td><td>1</td><td>444</td><td>3552</td><td>0.80%</td></tr>
<tr><th style='text-align:left'>WonderNewsMetadata</th><td>4</td><td>16</td><td>50.00%</td><td>1</td><td>4</td><td>16</td><td>0.00%</td></tr>
</tbody>
</table>

#### Counts by sector

| Typename | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | Total |
| :- | -: | -: | -: | -: | -: | -: | -: | -: | -: | -: | -: | -: | -: | -: | -: |
|**Apprentice**|4||||||||||||||4|
|**ApprenticeMon**|12||||||||||||||12|
|**ApprenticeQuestion**|9||||||||||||||9|
|**BattleDomeTrainer**|16||||||||||||||16|
|**BattleFrontier**|1||||||||||||||1|
|**BattleTowerEReaderTrainer**|1||||||||||||||1|
|**BattleTowerInterview**|1||||||||||||||1|
|**BattleTowerPokemon**|27||||||||||||||27|
|**Berry2**||||1|||||||||||1|
|**BerryCrush**|1||||||||||||||1|
|**BerryPickingResults**|1||||||||||||||1|
|**BerryTree**|||128||||||||||||128|
|**BoxPokemon**||6||2||54|54|54|54|54|54|54|42||428|
|**ContestWinner**||||13|||||||||||13|
|**Coords16**||49|||||||||||||49|
|**CustomGameOptions**|1||||||||||||||1|
|**CustomGameSavestate**|||||1||||||||||1|
|**DayCare**||||1|||||||||||1|
|**DaycareMail**||||2|||||||||||2|
|**DaycareMon**||||2|||||||||||2|
|**DewfordTrend**||||5|||||||||||5|
|**DomeMonData**|3||||||||||||||3|
|**EmeraldBattleTowerRecord**|6||||||||||||||6|
|**EnigmaBerry**||||1|||||||||||1|
|**ExternalEventData**||||1|||||||||||1|
|**ExternalEventFlags**||||1|||||||||||1|
|**GabbyAndTyData**||||1|||||||||||1|
|**ItemSlot**||236|||||||||||||236|
|**LilycoveLady**|||||1||||||||||1|
|**LilycoveLadyContest**|||||1||||||||||1|
|**LilycoveLadyFavor**|||||1||||||||||1|
|**LilycoveLadyQuiz**|||||1||||||||||1|
|**LinkBattleRecord**||||5|||||||||||5|
|**LinkBattleRecords**||||1|||||||||||1|
|**Mail**||||18|||||||||||18|
|**MauvilleManBard**||||1|||||||||||1|
|**MauvilleManCommon**||||1|||||||||||1|
|**MauvilleManGiddy**||||1|||||||||||1|
|**MauvilleManHipster**||||1|||||||||||1|
|**MauvilleManStoryteller**||||1|||||||||||1|
|**MauvilleOldManTrader**||||1|||||||||||1|
|**MysteryGiftSave**||||1|||||||||||1|
|**ObjectEvent**||16|||||||||||||16|
|**ObjectEventTemplate**||46|18||||||||||||64|
|**OldMan**||||1|||||||||||1|
|**PlayersApprentice**|1||||||||||||||1|
|**Pokeblock**||40|||||||||||||40|
|**Pokedex**|1||||||||||||||1|
|**Pokemon**||6|||||||||||||6|
|**PokemonJumpRecords**|1||||||||||||||1|
|**PokemonStorage**||||||1|||||||||1|
|**PokemonSubstruct0**||6||2||54|54|54|54|54|54|54|42||428|
|**PokemonSubstruct1**||6||2||54|54|54|54|54|54|54|42||428|
|**PokemonSubstruct2**||6||2||54|54|54|54|54|54|54|42||428|
|**PokemonSubstruct3**||6||2||54|54|54|54|54|54|54|42||428|
|**PokeNews**||||16|||||||||||16|
|**PyramidBag**|1||||||||||||||1|
|**RamScript**|||||1||||||||||1|
|**RamScriptData**|||||1||||||||||1|
|**RankingHall1P**|54||||||||||||||54|
|**RankingHall2P**|6||||||||||||||6|
|**RecordMixingGift**|||||1||||||||||1|
|**RecordMixingGiftData**|||||1||||||||||1|
|**RentalMon**|6||||||||||||||6|
|**Roamer**||||1|||||||||||1|
|**SaveBlock1**||1|||||||||||||1|
|**SaveBlock2**|1||||||||||||||1|
|**SecretBase**|||14|6|||||||||||20|
|**SecretBaseParty**|||13|7|||||||||||20|
|**SerializedBoxPokemonSubstructs**||6||2||54|54|54|54|54|54|54|42||428|
|**Time**|2||||||||||||||2|
|**TrainerHillSave**|||||1||||||||||1|
|**TrainerNameRecord**|||||20||||||||||20|
|**TVShow**||||25|||||||||||25|
|**WaldaPhrase**|||||1||||||||||1|
|**WarpData**||5|||||||||||||5|
|**WonderCard**|||||1||||||||||1|
|**WonderCardMetadata**|||||1||||||||||1|
|**WonderNews**||||1|||||||||||1|
|**WonderNewsMetadata**|||||1||||||||||1|

#### Counts by top-level value

| Typename | gSaveBlock2Ptr | gCustomGameOptions | gSaveBlock1Ptr | gCustomGameSavestate | gPokemonStoragePtr | Total |
| :- | -: | -: | -: | -: | -: | -: |
|**Apprentice**|4|||||4|
|**ApprenticeMon**|12|||||12|
|**ApprenticeQuestion**|9|||||9|
|**BattleDomeTrainer**|16|||||16|
|**BattleFrontier**|1|||||1|
|**BattleTowerEReaderTrainer**|1|||||1|
|**BattleTowerInterview**|1|||||1|
|**BattleTowerPokemon**|27|||||27|
|**Berry2**|||1|||1|
|**BerryCrush**|1|||||1|
|**BerryPickingResults**|1|||||1|
|**BerryTree**|||128|||128|
|**BoxPokemon**|||8||420|428|
|**ContestWinner**|||13|||13|
|**Coords16**|||49|||49|
|**CustomGameOptions**||1||||1|
|**CustomGameSavestate**||||1||1|
|**DayCare**|||1|||1|
|**DaycareMail**|||2|||2|
|**DaycareMon**|||2|||2|
|**DewfordTrend**|||5|||5|
|**DomeMonData**|3|||||3|
|**EmeraldBattleTowerRecord**|6|||||6|
|**EnigmaBerry**|||1|||1|
|**ExternalEventData**|||1|||1|
|**ExternalEventFlags**|||1|||1|
|**GabbyAndTyData**|||1|||1|
|**ItemSlot**|||236|||236|
|**LilycoveLady**|||1|||1|
|**LilycoveLadyContest**|||1|||1|
|**LilycoveLadyFavor**|||1|||1|
|**LilycoveLadyQuiz**|||1|||1|
|**LinkBattleRecord**|||5|||5|
|**LinkBattleRecords**|||1|||1|
|**Mail**|||18|||18|
|**MauvilleManBard**|||1|||1|
|**MauvilleManCommon**|||1|||1|
|**MauvilleManGiddy**|||1|||1|
|**MauvilleManHipster**|||1|||1|
|**MauvilleManStoryteller**|||1|||1|
|**MauvilleOldManTrader**|||1|||1|
|**MysteryGiftSave**|||1|||1|
|**ObjectEvent**|||16|||16|
|**ObjectEventTemplate**|||64|||64|
|**OldMan**|||1|||1|
|**PlayersApprentice**|1|||||1|
|**Pokeblock**|||40|||40|
|**Pokedex**|1|||||1|
|**Pokemon**|||6|||6|
|**PokemonJumpRecords**|1|||||1|
|**PokemonStorage**|||||1|1|
|**PokemonSubstruct0**|||8||420|428|
|**PokemonSubstruct1**|||8||420|428|
|**PokemonSubstruct2**|||8||420|428|
|**PokemonSubstruct3**|||8||420|428|
|**PokeNews**|||16|||16|
|**PyramidBag**|1|||||1|
|**RamScript**|||1|||1|
|**RamScriptData**|||1|||1|
|**RankingHall1P**|54|||||54|
|**RankingHall2P**|6|||||6|
|**RecordMixingGift**|||1|||1|
|**RecordMixingGiftData**|||1|||1|
|**RentalMon**|6|||||6|
|**Roamer**|||1|||1|
|**SaveBlock1**|||1|||1|
|**SaveBlock2**|1|||||1|
|**SecretBase**|||20|||20|
|**SecretBaseParty**|||20|||20|
|**SerializedBoxPokemonSubstructs**|||8||420|428|
|**Time**|2|||||2|
|**TrainerHillSave**|||1|||1|
|**TrainerNameRecord**|||20|||20|
|**TVShow**|||25|||25|
|**WaldaPhrase**|||1|||1|
|**WarpData**|||5|||5|
|**WonderCard**|||1|||1|
|**WonderCardMetadata**|||1|||1|
|**WonderNews**|||1|||1|
|**WonderNewsMetadata**|||1|||1|

