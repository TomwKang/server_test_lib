#!/bin/bash
cd build
./tom_server_test mem nothread
echo -e "【磁盘顺序写入1GB测试(‘x’字符)】"
./tom_server_test disk write
echo -e "【清除内存缓存】"
sudo sh -c "sync && echo 3 > /proc/sys/vm/drop_caches"
echo -e "【磁盘顺序读取1GB测试】"
./tom_server_test disk read
cd ..
