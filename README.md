# IF3230-K04-nubesx100

Sistem Paralel dan Terdistribusi - Konvolusi Matriks

## Skema Paralelisasi

### OpenMP

Untuk menghitung nilai matriks konvolusi, for loop sepanjang baris dan kolom matriks output diurai menjadi loop sepanjang array yang di-flatten, kemudian dilakukan parallel for loop dengan num_threads sebesar thread_count. Untuk kemudian mencari range terbesar pada matriks konvolusi, matriks konvolusi diurai menjadi array flat kembali, kemudian untuk setiap elemen, jika lebih besar daripada max kini, maka update nilai max dalam critical section. Jika lebih kecil daripada min kini, maka update nilai min dalam critical section. Setelah selesai mengiterasi seluruh matriks konvolusi, nilai max dikurangi nilai min dikembalikan.

### OpenMPI

Matriks kernel, serta jumlah _thread_ yang digunakan di-_broadcast_ kepada semua proses. Setelah itu, jumlah matriks untuk setiap proses di-_scatter_ kepada setiap proses agar setiap proses dapat mengalokasi array berukuran jumlah tersebut yang sesuai. Dengan menggunakan ukuran tersebut, maka juga akan dilakukan _scatterv_ terhadap semua matriks yang ada sehingga dapat didistribusikan secara merata untuk setiap proses. Setiap proses kemudian akan mengalokasikan array berukuran jumlah matriks tadi untuk menampung hasil kalkulasi _range_ dengan menggunakan OpenMP. Array ini kemudian di-_sort_ secara lokal untuk masing-masing proses, sebelum nantinya akan dikirimkan melalui perintah _gatherv_ kembali kepada _root process_, yaitu proses dengan _rank_ 0. Pada proses 0, array yang menampung semua hasil kalkulasi tersebut akan diurutkan kembali, lalu hasil pengurutan ini merupakan hasil akhir yang akan menjadi bahan perhitungan statistik yang diperlukan.

## Analisis Eksekusi Terbaik

1. Cara Kerja Paralelisasi
   Paralelisasi dicapai menggunakan OpenMPI untuk distribusi matriks antara proses, dan OpenMP untuk distribusi beban antara thread. Skema paralelisasi diimplementasi sebagaimana dijelaskan pada bagian sebelumnya.

2. Waktu Eksekusi Terbaik
   Untuk TC2, TC3, dan TC4, eksekusi paralel selesai dalam waktu yang lebih depat. Namun speedup terbaik dicapai untuk TC4, dengan jumlah matriks 5000.

## Perbandingan Hasil Eksekusi Program Secara Serial dan Paralel

Berikut informasi hasil eksekusi :

```shell
TC1 => serial
TC2 => parallel
TC3 => parallel
TC4 => parallel
```

Dari informasi diatas, dapat dilihat bahwa semakin banyak jumlah matriks dan ukuran matriks yang
digunakan, akan meningkatkan performa pemograman parallel. Oleh karena itu kita dapat menyimpulkan
bahwa untuk kasus test case yang memiliki kapasitas kecil, program paralel tidak lebih baik dari
serial namun untuk test case yang memiliki kapasitas besar, program paralel akan jauh lebih
unggul jika dibandingkan dengan program serial.

## Variasi Eksekusi dan Pengaruh OpenMP dan OpenMPI

```shell
| TC  | Node OpenMPI | Thread OpenMP | Waktu Eksekusi (second) |
| --- | ------------ | ------------- | ----------------------- |
| 1   | 2            | 5             | 0.027230                |
| 1   | 2            | 16            | 0.030819                |
| 1   | 3            | 5             | 0.043672                |
| 1   | 3            | 16            | 0.029709                |
| 1   | 4            | 5             | 0.036623                |
| 1   | 4            | 16            | 0.037708                |
| 1S  | -            | -             | 0.008659                |
| 2   | 2            | 5             | 0.611569                |
| 2   | 2            | 16            | 0.625597                |
| 2   | 3            | 5             | 0.538054                |
| 2   | 3            | 16            | 0.525512                |
| 2   | 4            | 5             | 0.559031                |
| 2   | 4            | 16            | 0.494060                |
| 2S  | -            | -             | 0.698502                |
| 3   | 2            | 5             | 0.789968                |
| 3   | 2            | 16            | 0.686185                |
| 3   | 3            | 5             | 0.785868                |
| 3   | 3            | 16            | 1.368298                |
| 3   | 4            | 5             | 1.570319                |
| 3   | 4            | 16            | 0.835112                |
| 3S  | -            | -             | 0.677666                |
| 4   | 2            | 5             | 8.602319                |
| 4   | 2            | 16            | 8.239505                |
| 4   | 3            | 5             | 8.936908                |
| 4   | 3            | 16            | 10.688829               |
| 4   | 4            | 5             | 13.540155               |
| 4   | 4            | 16            | 9.101202                |
| 4S  | -            | -             | 13.078884               |
```

**S adalah waktu eksekusi program secara serial.**

Tabel di atas memperlihatkan perbandingan waktu untuk setiap variasi jumlah _node_ dan _thread_ pada setiap _testcase_ yang ada. Secara umum, dapat terlihat bahwa untuk jumlah _node_ yang sama, program rata-rata menjadi lebih lambat ketika jumlah _thread_ ditingkatkan. Hal ini dikarenakan terdapat _overhead_ yang lebih banyak antara setiap _thread_ yang melakukan pemrosesan, terutama terhadap variabel atau data yang diakses bersamaan. Hal serupa juga terjadi ketika untuk suatu jumlah _thread_ yang sama dan jumlah _node_ ditingkatkan, yaitu _overhead_ yang dihasilkan malah lebih besar akibat banyaknya proses yang bersifat _blocking_ untuk setiap fase pemrosesan.
Namun demikian, ketika ukuran data yang diproses bertambah, dapat terlihat secara jelas bahwa kecepatan pemrosesan meningkat signifikan dan waktu eksekusi program menjadi jauh lebih baik. Hal ini dapat terlihat pada tabel di atas, yang menunjukkan bahwa _testcase_ 1 dengan jumlah data yang sedikit malah memiliki waktu eksekusi yang lambat untuk program paralel jika dibandingkan dengan program serialnya, tetapi tidak untuk _testcase_ lainnya.

## Author

1. 13519180 Karel Renaldi
2. 13519185 Richard Rivaldo
3. 13519205 Muhammad Rifat Abiwardani
