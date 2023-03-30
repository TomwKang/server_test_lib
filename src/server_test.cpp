#include <iostream>
#include <chrono>
#include <cstring>
#include <fstream>
#include <sys/time.h>
#include <sys/resource.h>
#include "net_test.hpp"
using namespace std;

const long long MAX_SIZE = 1024 * 1024 * 1024;
long long max_size_new = 0;
long long max_size_malloc = 0;
long long mem_total = 0;

void getOsInfo()
{
    FILE *fp = fopen("/proc/version", "r");
    if (NULL == fp)
        printf("failed to open version\n");
    char szTest[1000] = {0};
    while (!feof(fp))
    {
        memset(szTest, 0, sizeof(szTest));
        fgets(szTest, sizeof(szTest) - 1, fp); // leave out \n
        printf("%s", szTest);
    }
    fclose(fp);
}

void getCpuInfo()
{
    string file_name = "/proc/cpuinfo";
    ifstream file(file_name);
    string line;
    while (getline(file, line))
    {
        if (line.find("model name") != string::npos)
        {
            std::cout << "CPU " << line.substr(line.find(" ")) << std::endl;
            break;
        }
    }
}

void getMemoryInfo()
{
    string file_name = "/proc/meminfo";
    ifstream file(file_name);
    string line;
    while (getline(file, line))
    {
        if (line.find("MemTotal:") != string::npos)
        {
            mem_total = stoll(line.substr(line.find(" ")));
            mem_total = mem_total / KB_SIZE;
            std::cout << "MemTotal: " << mem_total << " MB" << std::endl;
            break;
        }
    }
}

void MemTestNew(long long size)
{
    char *buffer = new char[size * MB_SIZE];
    delete[] buffer;
    buffer = nullptr;
}

void MemCopyTest(long long size)
{
    // 记录开始时间
    auto start = chrono::high_resolution_clock::now();
    char *buffer = new char[size * MB_SIZE];
    memset(buffer, 'a', size * MB_SIZE);
    delete[] buffer;
    buffer = nullptr;
    // 记录结束时间
    auto end = chrono::high_resolution_clock::now();
    // 计算耗时（毫秒）
    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start).count();
    // 输出结果
    cout << "测试分配大小: " << size << " MB, 用时: " << duration << " ms" << endl;
    cout << "速度: " << static_cast<double>(size) * 1000 / duration << " MB/s" << endl;
}

void MemTestMalloc(long long size)
{
    void *buffer = malloc(size * MB_SIZE);
    if (buffer)
    {
        free(buffer);
        buffer = nullptr;
    }
    else
    {
        throw std::runtime_error("malloc failed");
    }
}

void MemTestMaxNew()
{
    try
    {
        while (max_size_new < MAX_SIZE)
        {
            MemTestNew(max_size_new);
            ++max_size_new;
        }
    }
    catch (const std::exception &e)
    {
        // cout << "\n=====MemTestMaxNew=====" << endl;
        // std::cerr << e.what() << '\n';
        std::cout << "new 最大分配内存： " << static_cast<double>(max_size_new) / 1024 << " GB" << endl;
        // cout << "=====MemTestMaxNew=====\n"
        //  << endl;
    }
}

void MemTestMaxMalloc()
{
    try
    {
        while (max_size_malloc < MAX_SIZE)
        {
            MemTestMalloc(max_size_malloc);
            ++max_size_malloc;
        }
    }
    catch (const std::exception &e)
    {
        // cout << "\n=====MemTestMaxMalloc=====" << endl;
        // std::cerr << e.what() << '\n';
        std::cout << "malloc 最大分配内存： " << static_cast<double>(max_size_malloc) / 1024 << " GB" << endl;
        // cout << "=====MemTestMaxMalloc=====\n"
        //  << endl;
    }
}

void DiskReadTest()
{
    cout << "=============DiskReadTest=============" << endl;
    long long file_size = 0;
    // 记录开始时间
    auto start = chrono::high_resolution_clock::now();

    fstream file_io_stream("test.txt", ios::in | ios::binary);
    file_io_stream.tie(nullptr);
    if (file_io_stream.is_open())
    {
        file_io_stream.seekg(0, ios::end);
        file_size = file_io_stream.tellg();
        char *buffer = new char[file_size];
        file_io_stream.seekg(0, ios::beg);
        file_io_stream.read(buffer, file_size);
        delete[] buffer;
        buffer = nullptr;
    }
    else
    {
        cout << "open file failed" << endl;
    }
    file_io_stream.close();

    // 记录结束时间
    auto end = chrono::high_resolution_clock::now();
    // 计算耗时（毫秒）
    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start).count();
    // 输出结果
    cout << "   Read Size: " << file_size / GB_SIZE << " GB, Time: " << duration << " ms" << endl;
    cout << "   Read Speed: " << static_cast<double>(GB_SIZE / MB_SIZE) * 1000 / duration << " MB/s" << endl;
    cout << "=============DiskReadTest============="
         << endl;
}

void DiskWirteTest()
{
    cout << "=============DiskWirteTest=============" << endl;
    long long file_size = GB_SIZE;
    char *buffer = new char[file_size];
    memset(buffer, 'x', file_size);
    // 记录开始时间
    auto start = chrono::high_resolution_clock::now();

    fstream file_io_stream;
    file_io_stream.open("test.txt", ios::out | ios::binary);
    file_io_stream << buffer;
    file_io_stream.close();

    // 记录结束时间
    auto end = chrono::high_resolution_clock::now();
    // 计算耗时（毫秒）
    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start).count();
    // 输出结果
    cout << "   Write Size: " << file_size / GB_SIZE << " GB, Time: " << duration << " ms" << endl;
    cout << "   Write Speed: " << static_cast<double>(GB_SIZE / MB_SIZE) * 1000 / duration << " MB/s" << endl;

    delete[] buffer;
    buffer = nullptr;
    cout << "=============DiskWirteTest============="
         << endl;
}

int main(int argc, char const *argv[])
{
    if (strcmp(argv[1], "mem") == 0)
    {
        cout << "========INFO========" << endl;
        getOsInfo();
        getCpuInfo();
        getMemoryInfo();
        cout << "========INFO========\n"
             << endl;
        /*内存分配测试*/
        cout << "【内存分配测试】" << endl;
        cout << "========MEMTEST========" << endl;
        MemTestMaxNew();
        MemTestMaxMalloc();

        mem_total *= 0.9;
        cout << "【测试分配90%最大内存】" << endl;
        MemCopyTest(mem_total);

        if (strcmp(argv[2], "thread") == 0)
        {
            // int thread_num = 1;
            // double mem_size = 7.0;
            // thread_num = atoi(argv[3]);
            // mem_size = atof(argv[4]);
            // boost::thread_group thread_group_mem;
            // for (int i = 0; i < thread_num; ++i)
            // {
            //     thread_group_mem.create_thread(boost::bind(MemCopyTest, mem_size * 1024));
            // }
            // thread_group_mem.join_all();
            // cout << "========MEMTEST========\n"
            //      << endl;
        }
        else if (strcmp(argv[2], "nothread") == 0)
        {
            cout << "========MEMTEST========\n"
                 << endl;
            return 0;
        }
        else
        {
            std::cout << "Input Error!" << std::endl;
        }
    }
    else if (strcmp(argv[1], "disk") == 0)
    {
        /*磁盘IO测试*/
        if (strcmp(argv[2], "read") == 0)
        {
            DiskReadTest();
        }
        else if (strcmp(argv[2], "write") == 0)
        {
            DiskWirteTest();
        }
        else
        {
            std::cout << "Input Error!" << std::endl;
        }
    }
    else
    {
        std::cout << "Input Error!" << std::endl;
    }
    return 0;
}