#ifndef __DEVICE_IDE_H
#define __DEVICE_IDE_H
#include "stdint.h"
#include "./../lib/sync.h"
#include "./../lib/bitmap.h"
#include "./../lib/list.h"

#define  bool int
/* 分区结构 */
struct partition {
   uint32_t start_lba;		 // 起始扇区
   uint32_t sec_cnt;		 // 扇区数
   struct disk* my_disk;	 // 分区所属的硬盘
   struct list_elem part_tag;	 // 用于队列中的标记,这个是本分区的标记
   char name[8];		 // 分区名称
   struct super_block* sb;	 // 本分区的超级块,这个是超级块指针，用来站位
   // 为了减少低速磁盘的访问次数，文件系统通常以多个扇区为单位来读写磁盘，这多个扇区就是块
   struct bitmap block_bitmap;	 // 块位图,用来管理本分区所有块,我们的块大小由1个扇去组成，所以
   // 所以block_bitmap就是管理扇区的位图
   struct bitmap inode_bitmap;	 // i结点位图
   struct list open_inodes;	 // 本分区打开的i结点队列
};

/* 硬盘结构 */
struct disk {
   char name[8];			   // 本硬盘的名称，如sda等
   struct ide_channel* my_channel;	   // 此块硬盘归属于哪个ide通道
   uint8_t dev_no;			   // 本硬盘是主0还是从1
   struct partition prim_parts[4];	   // 主分区顶多是4个
   struct partition logic_parts[8];	   // 逻辑分区数量无限,但总得有个支持的上限,那就支持8个
};

/* ata通道结构 */
struct ide_channel {
   char name[8];		 // 本ata通道名称 
   uint16_t port_base;		 // 本通道的起始端口号
   uint8_t irq_no;		 // 本通道所用的中断号
   struct lock lock;		 // 通道锁
   bool expecting_intr;		 // 表示等待硬盘的中断
   struct semaphore disk_done;	 // 用于阻塞、唤醒驱动程序
   struct disk devices[2];	 // 一个通道上连接两个硬盘，一主一从,表示一个通道上面有两个硬盘
};

void intr_hd_handler(uint8_t irq_no);
void ide_init(void);
extern uint8_t channel_cnt;
extern struct ide_channel channels[];
extern struct list partition_list;
void ide_read(struct disk* hd, uint32_t lba, void* buf, uint32_t sec_cnt);
void ide_write(struct disk* hd, uint32_t lba, void* buf, uint32_t sec_cnt);
extern uint8_t channel_cnt;
extern struct ide_channel channels[];


#endif

