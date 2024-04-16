# npbnlp
unsupervised learning libraries for NLP

## Requirements
- later cmake 3.20.X
- boost
- icu4c
- openmp

## cmake & make
```
% git clone https://github.com/sbintuitions/npbnlp.git
% cd npbnlp
% cmake -DCMAKE_CXX_COMPILER=<g++ path> -DLIBRARY_PATH=<icu_library_path> -DCPLUS_INCLUDE_PATH=<icu_include_path> .
% make
```

### tokenize
```
% ./ma/src/ws
[Usage]./ma/src/ws [options]
[example]
./ma/src/ws --train file --model file_to_save --dic dicfile
./ma/src/ws --parse file --model modelfile --dic dicfile
[options]
-n, --word_order=int(default 2)
-m, --letter_order=int(default 20)
-e, --epoch=int(default 500)
-t, --threads=int(default 4)
-v, --vocab=int(means letter variations. default 5000)

% head sample
村山富市首相は年頭にあたり首相官邸で内閣記者会と###日会見し、社会党の新民主連合所属議員の離党問題について「政権に影響を及ぼすことにはならない。離党者がいても、その範囲にとどまると思う」と述べ、大量離党には至らないとの見通しを示した。
また、####年中の衆院解散・総選挙の可能性に否定的な見解を表明、##日召集予定の通常国会前の内閣改造を明確に否定した。
ロシア側は首都制圧の最終段階に入ったとみられる。
グロズヌイからの報道では、ロシア軍は激しい空爆と砲撃を加えた後、装甲車部隊が大統領官邸付近に進出。
当面、行政改革、とりわけ規制緩和、特殊法人の見直し、地方分権など大きな課題がある。
党内の議論や党関係者の意見は「保守#党論はよろしくない。市民の側に立った平和と民主主義を担う政党が必要」というものだ。
そういう方向を目指して努力しなければならない。
党内にそれほどの動揺はない。
村山政権に影響があるということにはならない。
離党がうわさされている人と新党作りについての考え方は違いがないと思っている

% ./ma/src/ws --model data/npylm.model --dic data/word.dic --train sample
% ./ma/src/ws --model data/npylm.model --dic data/word.dic --parse sample
村山富市:54131
首相:20004
は:20163
年頭:74555
に:21931
あたり:92576
首相官邸:42941
で:23266
内閣:31810
記者会と:67107
###日:69716
会見:57623
し:22712
、:10673
社会党:27263
の新民主連:20847
合:10615
...
```

### morphological analysis
```
% ./ma/src/ma                                                                
[Usage]./ma/src/ma [options]
[example]
./ma/src/ma --train file --model file_to_save --dic dicfile
./ma/src/ma --parse file --model modelfile --dic dicfile
[options]
-n, --word_order=int(default 1)
-m, --letter_order=int(default 20)
-l, --pos_order=int(default 2)
-k, --pos=int(default 50)
-e, --epoch=int(default 500)
-t, --threads=int(default 4)
-v, --vocab=int(means letter variations. default 5000)

% ./ma/src/ma --model data/phsmm.n3.model --dic data/phsmm.n3.dic --train sample
% ./ma/src/ma --model data/phsmm.n3.model --dic data/phsmm.n3.dic --parse sample
村山富市首相:21902:29
は:51154:42
年頭:90334:41
に:60045:2
あたり:23624:14
首相官邸:19967:43
で:90234:5
内閣:131164:31
記者:50790:8
会:93122:43
と:53707:38
###日:153560:46
会見:111945:17
し、:23330:8
社会党の:44772:36
新民主連合:34349:29
所属議員:123243:20
の:30971:18
離党:112226:8
問題:156446:30
について:68067:8
「:22856:47
政権:23325:13
に:60045:2
影響を及ぼ:71450:14
すこと:104967:29
に:60045:2
はならない:34542:13
。:58895:13
...
```

### part-of-speech taggging
```
% ./tg/src/tg 
[Usage]./tg/src/tg [options]
[example]
./tg/src/tg --train file --model file_to_save --dic dicfile
./tg/src/tg --parse file --model modelfile --dic dicfile
[options]
-n, --pos_order=int(default 2)
-m, --letter_order=int(default 20)
-k, --pos=int(default 50)
-e, --epoch=int(default 500)
-t, --threads=int(default 4)
-v, --vocab=int(means letter variations. default 5000)

% head tokenized_sample
村山 富市 首相 は 年頭 に あたり 首相 官邸 で 内閣 記者 会 と ### 日 会見 し 、 社会党 の 新 民主 連合 所属 議員 の 離党 問題 に ついて 「 政権 に 影響 を 及ぼす こと に は なら ない 。 離党 者 が いて も 、 その 範囲 に とどまる と 思う 」 と 述べ 、 大量 離党 に は 至ら ない と の 見通し を 示した 。 
また 、 #### 年 中 の 衆院 解散 ・ 総 選挙 の 可能 性 に 否定 的な 見解 を 表明 、 ## 日 召集 予定 の 通常 国会 前 の 内閣 改造 を 明確に 否定 した 。 
ロシア 側 は 首都 制圧 の 最終 段階 に 入った と み られる 。 
グロズヌイ から の 報道 で は 、 ロシア 軍 は 激しい 空爆 と 砲撃 を 加えた 後 、 装甲 車 部隊 が 大統領 官邸 付近 に 進出 。 
当面 、 行政 改革 、 とりわけ 規制 緩和 、 特殊 法人 の 見直し 、 地方 分権 など 大きな 課題 が ある 。 
党 内 の 議論 や 党 関係 者 の 意見 は 「 保守 # 党 論 は よろしく ない 。 市民 の 側 に 立った 平和 と 民主 主義 を 担う 政党 が 必要 」 と いう もの だ 。 
そういう 方向 を 目指して 努力 し なければ なら ない 。 
党 内 に それほど の 動揺 は ない 。 
村山 政権 に 影響 が ある と いう こと に は なら ない 。 
離党 が うわさ さ れて いる 人 と 新党 作り に ついて の 考え 方 は 違い が ない と 思って いる 。

% ./tg/src/tg --model ihmm.model --dic tg.dic --train tokenized_sample
% ./tg/src/tg --model ihmm.model --dic tg.dic --parse tokenized_sample
```

### chunking & named entity recognition
```
% ./tg/src/ch                                                          
[Usage]./tg/src/ch [options]
[example]
./tg/src/ch --train file --tokenizer npylm.model --model file_to_save --cdic dicfile --wdic word.dic
./tg/src/ch --parse file --tokenizer npylm.model --model modelfile --cdic dicfile --wdic word.dic
[options]
-n, --order=int(default 2)
-e, --epoch=int(default 500)
-t, --threads=int(default 4)

% ./tg/src/ne
[Usage]./tg/src/ne [options]
[example]
./tg/src/ne --train file --tokenizer npylm.model --wdic word.dic --model file_to_save --cdic chunk.dic
./tg/src/ne --parse file --tokenizer npylm.model --wdic word.dic --model modelfile --cdic chunk.dic
[options]
-n, --chunk_order=int(default 1)
-m, --word_order=int(default 3)
-l, --letter_order=int(default 20)
-k, --class=int(default 500)
-e, --epoch=int(default 500)
-t, --threads=int(default 4)
-v, --vocab=int(means letter variations. default 5000)
```

### grammar induction
```
 % ./pa/src/pa
[Usage]./pa/src/pa [options]
[example]
./pa/src/pa --train file --model file_to_save --dic dicfile
./pa/src/pa --parse file --model modelfile --dic dicfile
[options]
-m, --letter_order=int(default 20)
-k, --max_category=int(default 100)
-e, --epoch=int(default 500)
-t, --threads=int(default 4)
-v, --vocab=int(means letter variations. default 5000)
--dot=flag output in dot format for graphviz

% head sample 
村山 富市 首相 は 年頭 に あたり 首相 官邸 で 内閣 記者 会 と ### 日 会見 し 、 社会党 の 新 民主 連合 所属 議員 の 離党 問題 に ついて 「 政権 に 影響 を 及ぼす こと に は なら ない 。 離党 者 が いて も 、 その 範囲 に とどまる と 思う 」 と 述べ 、 大量 離党 に は 至ら ない と の 見通し を 示した 。 
また 、 #### 年 中 の 衆院 解散 ・ 総 選挙 の 可能 性 に 否定 的な 見解 を 表明 、 ## 日 召集 予定 の 通常 国会 前 の 内閣 改造 を 明確に 否定 した 。 
ロシア 側 は 首都 制圧 の 最終 段階 に 入った と み られる 。 
グロズヌイ から の 報道 で は 、 ロシア 軍 は 激しい 空爆 と 砲撃 を 加えた 後 、 装甲 車 部隊 が 大統領 官邸 付近 に 進出 。 
当面 、 行政 改革 、 とりわけ 規制 緩和 、 特殊 法人 の 見直し 、 地方 分権 など 大きな 課題 が ある 。 
党 内 の 議論 や 党 関係 者 の 意見 は 「 保守 # 党 論 は よろしく ない 。 市民 の 側 に 立った 平和 と 民主 主義 を 担う 政党 が 必要 」 と いう もの だ 。 
そういう 方向 を 目指して 努力 し なければ なら ない 。 
党 内 に それほど の 動揺 は ない 。 
村山 政権 に 影響 が ある と いう こと に は なら ない 。 
離党 が うわさ さ れて いる 人 と 新党 作り に ついて の 考え 方 は 違い が ない と 思って いる 。 

% ./pa/src/pa --train sample --model ipcfg.model --dic grammar.dic
% ./pa/src/pa --parse sample --model ipcfg.model --dic grammar.dic
```
