# 關於

這是一個純粹 C++ 輸出到 stdout 的 conway life 模擬

計畫是可以使用其他工具(不管你用 GTK 還是什麼)輸出到你/妳喜歡的 GUI 介面

當然，你也可以直接使用本程式輸出結果。


# 語法

在程式開始之前，你必須給他一個"寬"、"高"

而，其所產生的地圖，將會是以
```
'.' 當作死亡
```
```
'O' 當作存活
```

# 結束

當與歷史有出現過相同的場景時將會結束

(場地淨空的下一個狀態也會是淨空，也就會視為歷史有出現過)

也就是這邊如果偵測到迴圈(重複狀態)即結束本程式。