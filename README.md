# IF3230-K04-nubesx100

Sistem Paralel dan Terdistribusi - Konvolusi Matriks

## Skema Paralelisasi

### OpenMP

### OpenMPI

## Analisis Eksekusi Terbaik

## Perbandingan Hasil Eksekusi Program Secara Serial dan Paralel

## Variasi Eksekusi dan Pengaruh OpenMP dan OpenMPI

## Catatan Eksekusi

### Langkah-Langkah Menggunakan Cluster Server dengan SSH

1. Lakukan SSH kepada salah satu server dengan public IP. Pilihlah satu server saja (lakukan satu command saja), command sebagai berikut:

```
ssh -i ~/.ssh/k04-02 k04-02@35.240.188.57
ssh -i ~/.ssh/k04-02 k04-02@34.87.108.45
ssh -i ~/.ssh/k04-02 k04-02@34.124.168.121
ssh -i ~/.ssh/k04-02 k04-02@35.198.245.105
```

SSH dilakukan dengan menggunakan private key yang sudah didownload. Taruhlah private key di dalam folder ~/.ssh yang ada di home. Cara SSH dengan private key tiap os mungkin berbeda.

2. Di dalam remote machine dari SSH di perintah sebelumnya, lakukan SSH terhadap machine lain dengan menggunakan private ip. Jalankan semua command dibawah ini, kecuali private ip dari mesin sekarang. (misal di step sebelumnya ssh server ke-2 maka sekarang hanya perlu ssh ke server 1,3,4)

```
ssh 10.148.0.22
ssh 10.148.0.23
ssh 10.148.0.21
ssh 10.148.0.20
```

3. Copy seluruh folder yang ada di local ke semua remote machine dengan menjalankan command dibawah.

```
scp -r -i ~/.ssh/k04-02 ./test k04-02@35.240.188.57:/home/k04-02/
scp -r -i ~/.ssh/k04-02 ./test k04-02@34.87.108.45:/home/k04-02/
scp -r -i ~/.ssh/k04-02 ./test k04-02@34.124.168.121:/home/k04-02/
scp -r -i ~/.ssh/k04-02 ./test k04-02@35.198.245.105:/home/k04-02/
```

4. Lakukan kompilasi kode. Lakukan di setiap instance mesin (4 mesin remote) dengan menggunakan command dibawah.

```shell
mpicc -o ./bin/.o ./src/<nama-file-1>.c -fopenmp -lm -c \
    && mpicc -o ./bin/<nama-file-2>.o ./src/<nama-file-2>.c -fopenmp -lm -c \
    && mpicc -o ./bin/<nama-file-3>.o ./src/<nama-file-3>.c -fopenmp -lm -c \
    && mpicc -o ./bin/<nama-file-final> ./bin/<nama-file-1>.o ./bin/matrix.o ./bin/utils.o ./bin/mpi_utils.o -fopenmp
```

5. Menjalankan Kode program yang telah dikompilasi.

Menjalankan kode test.

```
mpirun -np {banyak node} --hostfile ./hostname ./bin/main-test {banyak thread}
mpirun -np 2 --hostfile ./hostname ./bin/main-test 2
```
