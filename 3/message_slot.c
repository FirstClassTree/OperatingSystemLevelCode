/*Took reciation code as skeleton*/


#undef __KERNEL__
#define __KERNEL__
#undef MODULE
#define MODULE


#include <linux/kernel.h>  
#include <linux/module.h>   
#include <linux/fs.h>       
#include <linux/uaccess.h>  
#include <linux/string.h>   
#include <linux/slab.h> 


MODULE_LICENSE("GPL");

#include "message_slot.h"

typedef struct channel {
    unsigned int channel_id;
    int message_length;
    char message[BUF_LEN];
    struct channel *next;
} channel;
/* minor from 1 - 256*/
static channel *channelHeadsForMinorKey[257]; 

// The message the device will give when asked


//================== DEVICE FUNCTIONS ===========================
static int device_open( struct inode* inode,
                        struct file*  file )
{
  
  return SUCCESS;
}


//---------------------------------------------------------------

static ssize_t device_read( struct file* file,
                            char __user* buffer,
                            size_t       length,
                            loff_t*      offset )
{
    int i;
    int minorNum = iminor(file->f_inode); /*getting the minor key*/
    channel *currChannel = (channel*)file->private_data;
    if(currChannel == NULL){
      printk(KERN_ERR "channel not initialzied or empty, read");
      return -EINVAL;
    }
    printk(KERN_ERR "read,channel id:%d ,minorNum:%d,message_length:%d",currChannel->channel_id,minorNum,currChannel->message_length);
    for(i = 0; i < currChannel->message_length;i++){
        printk(KERN_ERR "%c",currChannel->message[i]);
    }
    if(buffer == NULL){
      printk(KERN_ERR "buffer is NULL, read");
      return -ENOSPC;
    }
    if(currChannel->message_length == 0){
      printk(KERN_ERR "no message, read");
        return -EWOULDBLOCK;
    }
    if(length < currChannel->message_length){
        printk(KERN_ERR "message too long, read");
        return -ENOSPC;
    }
     for(i = 0; i < currChannel->message_length ; i++) {
        if(0 != put_user(currChannel->message[i], &buffer[i]) ) {
          printk(KERN_ERR "failure to read adress, read");
            return -EFAULT; 
        }
    }
    printk(KERN_INFO "read message in channel %d minor %d", currChannel->channel_id,minorNum);
    return i;
  
}

//---------------------------------------------------------------

static ssize_t device_write( struct file*       file,
                             const char __user* buffer,
                             size_t             length,
                             loff_t*            offset)
{
  int i;
  char tempBuffer[BUF_LEN];
  int minorNum = iminor(file->f_inode); 
  channel *currChannel = (channel *)file->private_data;
  
  if(currChannel == NULL){
    printk(KERN_ERR "no initialzieion, write");
    return -EINVAL;
  }
  if(buffer == NULL){
    printk(KERN_WARNING "NULL Buffer, write");
    return -EINVAL;
  }
  if(length == 0 || length > BUF_LEN ){
    printk(KERN_ERR "improper length, write");

    return -EMSGSIZE;
  }
  
  for(i = 0; i <length; i++){
    if(0 != get_user(tempBuffer[i], &buffer[i])){
          printk(KERN_ERR "failure to write to adresss, write");
        return -EFAULT;
    }
  }
  currChannel->message_length = i;
  for(i = 0; i < currChannel->message_length; i++){
    currChannel->message[i] = tempBuffer[i];
     printk(KERN_ERR "%c",currChannel->message[i]);
  }
  
  printk(KERN_INFO "%d" ,    i);
  printk(KERN_INFO "write message in channel %d minor %d", currChannel->channel_id,minorNum);
  printk(KERN_INFO "write message length %d", currChannel->message_length);
  file->private_data = currChannel;
  return i;
}

//----------------------------------------------------------------
static long device_ioctl( struct   file* file,
                          unsigned int   ioctl_command_id,
                          unsigned long  ioctl_param )
{
   
  bool found = false;
  int minorNum;
  channel *new_channel;
  channel *temp_channel;
  
  // Switch according to the ioctl called
  if(ioctl_command_id == MSG_SLOT_CHANNEL  ) {

     /*as stated in assigmnet*/
    if(ioctl_param == 0){
          printk(KERN_ERR "bad channel id parmater 0, ioctl");
        return -EINVAL;
    }

    minorNum = iminor(file->f_inode); /*getting the minor key*/
    temp_channel = channelHeadsForMinorKey[minorNum];
    /*getting one before last in list, and adding to it, if don't exist it is the head and allocating*/
    if(temp_channel != NULL){
      while(temp_channel->next != NULL){
        if(temp_channel->channel_id == ioctl_param){
          file->private_data = temp_channel;
          found = true;
          return SUCCESS;
        }
        temp_channel = temp_channel->next;
      }
       if(temp_channel->channel_id == ioctl_param){
          file->private_data = temp_channel;
          found = true;
          return SUCCESS; 
        }
      if(found == false){
        new_channel = kmalloc(sizeof(channel),GFP_KERNEL);
        if(new_channel == NULL){
            printk(KERN_ERR "failure to alloc, ioctl");
          return -ENOMEM;
        }
        new_channel->channel_id = ioctl_param;
        new_channel->message_length = 0;
        new_channel->next = NULL;
        memset(new_channel->message, 0, BUF_LEN); 
        temp_channel->next = new_channel;
        file->private_data = new_channel;
      }
    }
    else{
      new_channel = kmalloc(sizeof(channel),GFP_KERNEL);
        if(new_channel == NULL){
                printk(KERN_ERR "failure to alloc, ioctl");
          return -ENOMEM;
        }
      
      new_channel->channel_id = ioctl_param;
      memset(new_channel->message, 0, BUF_LEN); 
      new_channel->message_length = 0;
      new_channel->next = NULL;
      file->private_data = new_channel;
      channelHeadsForMinorKey[minorNum] = new_channel;
    }
    new_channel = (channel*)file->private_data;
    printk(KERN_INFO "ioctl in channel %d minor %d", new_channel->channel_id,minorNum);
    return SUCCESS;
    }

    return -EINVAL;
    
  
}
static int slot_release(struct inode *inode, struct file *file) {
  /*in my implemntiaon nothing is allocated for a file descriptor*/
    return SUCCESS; 
}

//==================== DEVICE SETUP =============================
struct file_operations Fops = {
  .owner	  = THIS_MODULE, 
  .read           = device_read,
  .release = slot_release,
  .write          = device_write,
  .open           = device_open,
  .unlocked_ioctl = device_ioctl,
};

//---------------------------------------------------------------

static int __init slot_init(void)
{
  int i;
  int rc = -1;
  /*initiailza the heads of the channels*/
  for(i =0; i< 257; i++){
    channelHeadsForMinorKey[i] = NULL;
  }


  
  rc = register_chrdev( MAJOR_NUM, DEVICE_FILE_NAME, &Fops );
  printk(KERN_INFO "%s: slot module loaded with device major number %d\n", DEVICE_RANGE_NAME, MAJOR_NUM);

  if( rc < 0 ) {
    printk(KERN_ERR "Initizilaztion failure");
    return rc;
  }

  return 0;
}

//---------------------------------------------------------------
static void __exit slot_cleanup(void)
{
  int i;
  channel *tempChannel;
  channel *curChannel;
  
  for(i =0; i <257; i++){
    curChannel = (channel*)channelHeadsForMinorKey[i];
    tempChannel = curChannel;
    while(curChannel != NULL){
      curChannel = curChannel->next;
      kfree(tempChannel);
      tempChannel = curChannel;
    }
  }
  unregister_chrdev(MAJOR_NUM, DEVICE_RANGE_NAME);
}

//---------------------------------------------------------------
module_init(slot_init);
module_exit(slot_cleanup);

//========================= END OF FILE =========================
