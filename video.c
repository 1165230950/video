#include "head.h"

int fd;
unsigned char *buffer;
unsigned char *rgb_buffer;
int buf_size;
VideoBuffer framebuf;

/*===========================进入主函数===========================*/
int main(int argc, char *argv[])
{
	/*----------------打开视频设备------------*/
	framebuf.dev = open(CAMERA_DEVICE, O_RDWR);
	if(framebuf.dev < 0){
		printf("Open %s falsed\n", CAMERA_DEVICE);
		return -1;
	}

	/*-----------------获取驱动信息-----------*/
	int ret;
	ret = get_driver_massage(&framebuf);

	/*-------------------显示所有支持的格式-----------------------*/

	ret = show_any_support_format(&framebuf);

	/*---------------------查看当前格式---------------------------*/

	ret = show_now_format(&framebuf);

	/*---------------------设置视频格式---------------------------*/
						
	ret = set_video_format(&framebuf);

	/*---------------------获取视频格式---------------------------*/

	ret = get_video_format(&framebuf);

	/*---------------------请求分配内存---------------------------*/

	ret = request_buffer(&framebuf);

	/*-------------------获取缓冲帧的信息-------------------------*/
			
	ret = get_v4l2_buffer_info(&framebuf);
	
	/*------------------映射地址到用户空间------------------------*/
	
	buffer = mapp_buffer(&framebuf);

	/*--------------------把缓冲帧放入缓冲队列--------------------*/
		
	ret = ioctl(framebuf.dev , VIDIOC_QBUF, &framebuf.buf);
	if (ret < 0) {
		printf("VIDIOC_QBUF (%d) failed (%d)\n", 0, ret);
		return -1;
	}
	printf("Frame buffer %d : address=0x%x, length=%d\n", 0, framebuf.start, (int)framebuf.length);

	/*--------------------------开始录制--------------------------*/
	printf("----------- get the info of V4L2 buffer -------------\n");
	
	enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	
	ret = ioctl(framebuf.dev, VIDIOC_STREAMON, &type); //开始视频采集
	if (ret < 0) {
		printf("VIDIOC_STREAMON failed (%d)\n", ret);
		return -1;
	}

	/*------------------------从队列取出数据-------------------------*/
	ret = ioctl(framebuf.dev, VIDIOC_DQBUF, &framebuf.buf);
	if(ret < 0){
		printf("VIDIOC_DQBUF failed (%d)\n", ret);
		return 0;
	}
												
	memset(buffer,0,framebuf.length);
	memcpy(buffer, framebuf.start, framebuf.buf.bytesused);

	buf_size = framebuf.buf.bytesused;
	
	/*-------------------把缓冲帧重新插入缓冲队列--------------------*/
	ret = ioctl(framebuf.dev, VIDIOC_QBUF, &framebuf.buf);
	if(ret < 0){
		printf("VIDIOC_QBUF failed (%d)\n", ret);
	}
	
	yuyv_to_rgb(buffer, rgb_buffer);


	munmap(framebuf.start,framebuf.length);
	//关闭设备
	close(framebuf.dev);
	printf("Camera test Done.\n");
	
	return 0;
}
