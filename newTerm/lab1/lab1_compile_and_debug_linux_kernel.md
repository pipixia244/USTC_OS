# 实验一：

# 编译运行Linux内核并通过qemu+gdb调试

## 实验目的

* 熟悉Linux系统运行环境
* 制作根文件系统
* 掌握Linux内核编译方法
* 学习如何使用gdb调试内核
* 熟悉Linux下常用的文件操作指令

## 实验环境

* OS：Ubuntu 16.04 LTS/18.04 LTS/20.04 LTS（64 bit）
* 编译调试Linux内核版本：Kernel 4.9.263
* **注意**：本次实验必须在Ubuntu系统上实现，可直接在机房完成，或在个人PC上安装虚拟环境完成，请注意需要下载安装64位的Ubuntu镜像。
## 实验内容
### 一、制作根文件系统
#### 1、下载并编译Linux内核
* 下载linux-4.9.263.tar.gz，解压缩得到目录linux-4.9.263，不妨称之为Linux源代码根目录(以下简称源码根目录)
  ```shell
  mkdir ~/oslab
  cd ~/oslab
  wget https://cdn.kernel.org/pub/linux/kernel/v4.x/linux-4.9.263.tar.xz #下载
  tar -zvxf linux-4.9.263.tar.xz #解压
  ```

* 准备安装所需依赖库
  * 可能ubuntu官方镜像源安装下载镜像速度较慢，将镜像源切换成ustc源即可，具体方法见下
  * [更换apt-get源为ustc镜像源](http://mirrors.ustc.edu.cn/help/ubuntu.html)
   ```shell
  sudo apt-get install git build-essential  xz-utils libssl-dev bc libncurses5-dev libncursesw5-dev
  ```

* 进入源代码根目录，并编译配置选择 (本次实验直接选择Save,然后exist)
* [内核配置(make menuconfig)详述](https://blog.csdn.net/fanle76/article/details/52330265)
  ```shell
  cd ~/oslab/linux-4.9.263
  make menuconfig
  ```
* 执行编译指令
  ```shell
  make -j $((`nproc`-1))
  ```

* make过程中若遇到问题，可参考以下方案解决：
  ```shell
    # 问题1
    *******************
    
    # 问题1解决方案：
    *******************
  ```


#### 2、准备模拟器qemu
* 直接安装qemu包即可

  ```shell
  sudo apt install qemu
  ```


#### 3、制作根文件系统：利用busybox生成根文件系统

* （1）下载busybox

  ```shell
    cd ~/oslab
    wget https://busybox.net/downloads/busybox-1.32.1.tar.bz2  #下载
    tar -jxvf busybox-1.32.1.tar.bz2    #解压
    cd ~/oslab/busybox-1.32.1
    ```

* （2）编译busybox

  ```shell
  make menuconfig  #修改配置如下：  
  ```
  修改配置如下：(空格键勾选)
  ```shell
  Settings –>
    Build Options
     [*] Build static binary（no share libs）
  ```
  编译并安装
  ```shell
  make -j $((`nproc`-1))
  sudo make install
  ```
  


* （3）准备根文件系统

  ```shell
  cd ~/oslab/busybox-1.32.1/_install
  sudo mkdir dev 
  sudo mknod dev/console c 5 1
  sudo mknod dev/ram b 1 0 
  touch init
  ```
  在init中写入以下内容（你可以使用vim或gedit编辑器写入，或在图形化界面中找到该文件，双击编辑）
  ```
  #!/bin/sh
  echo "INIT SCRIPT"
  mkdir /proc
  mkdir /sys
  mount -t proc none /proc
  mount -t sysfs none /sys
  mkdir /tmp
  mount -t tmpfs none /tmp
  echo -e "\nThis boot took $(cut -d' ' -f1 /proc/uptime) seconds\n"
  exec /bin/sh
  ```

  赋予init脚本执行，制作initramfs文件，将x86-busybox下面的内容打包归档成cpio文件，以供Linux内核做initramfs启动执行
  ```
  chmod +x init
  cd ~/oslab/busybox-1.32.1/_install
  find . -print0 | cpio --null -ov --format=newc | gzip -9 > ~/oslab/initramfs-busybox-x64.cpio.gz  # 注意：该命令一定要在busybox的 _install 目录下执行
  # 注意：每次修改_install,都要重新执行该命令
  ```

* （4）运行

  ```shell
  cd ~/oslab
  qemu-system-x86_64 -s -kernel ~/oslab/linux-4.9.263/arch/x86_64/boot/bzImage -initrd ~/oslab/initramfs-busybox-x64.cpio.gz --append "root=/dev/ram init=/init"
  ```

  * 在qemu窗口可以看到成功运行，且进入shell环境

#### 4、熟悉linux简单指令

* 目标：掌握ls、touch、cat、echo、mkdir、mv、cd、cp等基本指令

* 在上一步“利用busybox生成根文件系统” 运行成功之后，在qemu窗口可以看到已进入shell环境。此时就可以在我们自己制作的根文件系统中执行指令了。如下指令创建写入一个txt文件并移动文件：

  ```shell
  ls          # 查看当前目录下的所有文件/文件夹
  touch 1.txt # 创建1.txt
  ls
  echo i am 1.txt > 1.txt  # 向1.txt写入内容
  cat 1.txt   # 查看1.txt内容
  ls -l       # 查看当前目录下的所有文件/文件夹的详细信息
  mkdir 1     # 创建目录1
  mv 1.txt 1  # 将1.txt移动到目录1
  cd 1        # 打开目录1
  ls
  ```



### 二、gdb+qemu调试内核

#### 1、gdb简介
* gdb是一款终端环境下常用的调试工具

* 使用gdb调试程序
    * ubuntu下安装gdb：sudo apt install gdb
    * 编译程序时加入-g选项，如：gcc -g -o test test.c
    * 运行gdb调试程序：gdb test

* 常用命令
  ``` shell
  r/run                 # 开始执行程序
  b/break <location>    # 在location处添加断点，location可以是代码行数或函数名
  b/break <location> if <condition> # 在location处添加断点，仅当caondition条件满足才中断运行
  c/continue            # 继续执行到下一个断点或程序结束
  n/next                # 运行下一行代码，如果遇到函数调用直接跳到调用结束
  s/step                # 运行下一行代码，如果遇到函数调用则进入函数内部逐行执行
  ni/nexti              # 类似next，运行下一行汇编代码（一行c代码可能对应多行汇编代码）
  si/stepi              # 类似step，运行下一行汇编代码
  list                  # 显示当前行代码
  p/print <expression>  # 查看表达式expression的值
  ```

#### 2、在qemu中启动gdb server
* 在终端中执行以下指令启动qemu运行内核：
  ```shell
  qemu-system-x86_64 -s -S -kernel ~/oslab/linux-4.9.263/arch/x86_64/boot/bzImage -initrd ~/oslab/initramfs-busybox-x64.cpio.gz --append "root=/dev/ram init=/init" # 可以看到qemu在等待gdb连接
  ```
* 关于-s和-S选项的说明
    * -S freeze CPU at startup (use ’c’ to start execution)
    * -s shorthand for -gdb tcp::1234 若不想使用1234端口，则可以使用-gdb tcp:xxxx来取代-s选项
#### 3、建立gdb与gdb server之间的链接
* 在另外一个终端运行gdb，然后在gdb界面中运行如下命令：
  ```shell
  gdb				   #这里一定是在另外一个终端运行，不能在qemu的窗口上输入
  target remote:1234 #则可以建立gdb和gdbserver之间的连接
  c                  #让qemu上的Linux继续运行
  ```
  可以看到gdb与qemu已经建立了连接。但是由于没有加载符号表，无法根据符号设置断点。下面说明如何加入断点。

#### 4、加载vmlinux中的符号表并设置断点

* 退出之前打开的qemu终端，重新执行第2步 ”在qemu中启动gdb server “

* 在另外一个终端输入如下指令运行gdb，加载符号表

  ```shell
  gdb				   #这里一定是在另外一个终端运行，不能在qemu的窗口上输入
  file ~/oslab/linux-4.9.263/vmlinux   #加载符号表
  target remote:1234 #建立gdb和gdbserver之间的连接
  ```

* 在gdb界面中设置断点

  ```shell
  break start_kernel
  c                  #继续运行到断点
  ```
#### 5、重新配置Linux，使之携带调试信息
* 在原来配置的基础上，重新配置Linux，使之携带调试信息

* ```shell
  cd ~/oslab/linux-4.9.263/
  make menuconfig
  	kernel hacking—>
  		[*] compile the kernel with debug info
  ```

* 重新编译并按照原方法执行

  ```shell
  make -j $((`nproc`-1))
  ```

* 此时，若按照前面相同的方法来运行，则在start_kernel停下 来后，可以使用list来显示断点处相关的源代码



## 参考资料
* [内核编译与制作根文件系统](https://www.centennialsoftwaresolutions.com/post/build-the-linux-kernel-and-busybox-for-arm-and-run-them-on-qemu)