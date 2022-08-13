# HM55 芯片组 Win10 补丁
解决 HM55 芯片组无法启动 Win8.1 及以后系统的问题  
感谢 [Treeyan/h33yfix](https://github.com/Treeyan/h33yfix) 提供的灵感  
该补丁会作为 mbr 写入磁盘，检查 pci 设备判断是否为 HM55 芯片组，查找 DSDT 表并对 ECFL 初值进行修改。  

# 使用方法  
编译或从 Release 下载 mbr.bin，将其写入 mbr，一共八个扇区。  
