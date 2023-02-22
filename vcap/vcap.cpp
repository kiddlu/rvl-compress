#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <getopt.h> /* getopt_long() */
#include <fcntl.h> /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <dlfcn.h>
#include <pthread.h>

#include <linux/videodev2.h>


#define SAVE_IMAGE
//#define HAVE_OPENCV

#ifdef HAVE_OPENCV
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#endif

struct buffer {
        void *start;
        size_t length;
        struct v4l2_buffer v4l2_buf;
};

#define BUFFER_COUNT 4
#define FMT_NUM_PLANES 1
#define CLEAR(x) memset(&(x), 0, sizeof(x))

#define DBG(...) do { if(!silent) printf(__VA_ARGS__); } while(0)
#define ERR(...) do { fprintf(stderr, __VA_ARGS__); } while (0)

static char *dev_name;
static char *dump_path;
static int width;
static int height;
static enum v4l2_buf_type buf_type;
static int format;

static int fd = -1;
static unsigned int n_buffers;
struct buffer *buffers;
static int silent=0;
FILE *fp=NULL;

static void errno_exit(const char *s)
{
	ERR("%s error %d, %s\n", s, errno, strerror(errno));
	exit(EXIT_FAILURE);
}

static int xioctl(int fh, int request, void *arg)
{
	int r;
	do {
			r = ioctl(fh, request, arg);
	} while (-1 == r && EINTR == errno);
	return r;
}

static void open_device(void)
{
    fd = open(dev_name, O_RDWR /* required */ /*| O_NONBLOCK*/, 0);

    if (-1 == fd) {
        ERR("Cannot open '%s': %d, %s\n",
                    dev_name, errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
}


static void init_display_buf(int buffer_size, int width, int height)
{
#ifdef HAVE_OPENCV 
	cv::namedWindow("video");
#endif
}


static void init_mmap(void)
{
	struct v4l2_requestbuffers req;

	CLEAR(req);

	req.count = BUFFER_COUNT;
	req.type = buf_type;
	req.memory = V4L2_MEMORY_MMAP;

	if (-1 == xioctl(fd, VIDIOC_REQBUFS, &req)) {
		if (EINVAL == errno) {
				ERR("%s does not support "
								"memory mapping\n", dev_name);
				exit(EXIT_FAILURE);
		} else {
				errno_exit("VIDIOC_REQBUFS");
		}
	}

	if (req.count < 2) {
		ERR("Insufficient buffer memory on %s\n",
						dev_name);
		exit(EXIT_FAILURE);
	}

	buffers = (struct buffer*)calloc(req.count, sizeof(*buffers));

	if (!buffers) {
		ERR("Out of memory\n");
		exit(EXIT_FAILURE);
	}

	for (n_buffers = 0; n_buffers < req.count; ++n_buffers) {
		struct v4l2_buffer buf;
		struct v4l2_plane planes[FMT_NUM_PLANES];
		CLEAR(buf);
		CLEAR(planes);

		buf.type = buf_type;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = n_buffers;

		if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == buf_type) {
			buf.m.planes = planes;
			buf.length = FMT_NUM_PLANES;
		}

		if (-1 == xioctl(fd, VIDIOC_QUERYBUF, &buf))
			errno_exit("VIDIOC_QUERYBUF");

		if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == buf_type) {
			buffers[n_buffers].length = buf.m.planes[0].length;
			buffers[n_buffers].start =
			mmap(NULL /* start anywhere */,
					buf.m.planes[0].length,
					PROT_READ | PROT_WRITE /* required */,
					MAP_SHARED /* recommended */,
					fd, buf.m.planes[0].m.mem_offset);
		} else {
			buffers[n_buffers].length = buf.length;
			buffers[n_buffers].start =
			mmap(NULL /* start anywhere */,
					buf.length,
					PROT_READ | PROT_WRITE /* required */,
					MAP_SHARED /* recommended */,
					fd, buf.m.offset);
		}

		if (MAP_FAILED == buffers[n_buffers].start)
				errno_exit("mmap");
	}
}

static void init_device(void)
{
    struct v4l2_capability cap;
    struct v4l2_format fmt;

    if (-1 == xioctl(fd, VIDIOC_QUERYCAP, &cap)) {
            if (EINVAL == errno) {
                    ERR("%s is no V4L2 device\n",
                                dev_name);
                    exit(EXIT_FAILURE);
            } else {
                    errno_exit("VIDIOC_QUERYCAP");
            }
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) &&
            !(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE)) {
        ERR("%s is not a video capture device, capabilities: %x\n",
                        dev_name, cap.capabilities);
            exit(EXIT_FAILURE);
    }

    if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
            ERR("%s does not support streaming i/o\n",
                dev_name);
            exit(EXIT_FAILURE);
    }

    if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)
        buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    else if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE)
        buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;

    CLEAR(fmt);
    fmt.type = buf_type;
    fmt.fmt.pix.width = width;
    fmt.fmt.pix.height = height;
    fmt.fmt.pix.pixelformat = format;
    fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;

    if (-1 == xioctl(fd, VIDIOC_S_FMT, &fmt))
            errno_exit("VIDIOC_S_FMT");

    init_mmap();

    init_display_buf(fmt.fmt.pix.sizeimage, width, height);
}


static void start_capturing(void)
{
	unsigned int i;
	enum v4l2_buf_type type;

	for (i = 0; i < n_buffers; ++i) {
			struct v4l2_buffer buf;

			CLEAR(buf);
			buf.type = buf_type;
			buf.memory = V4L2_MEMORY_MMAP;
			buf.index = i;

			if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == buf_type) {
				struct v4l2_plane planes[FMT_NUM_PLANES];

				buf.m.planes = planes;
				buf.length = FMT_NUM_PLANES;
			}
			if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
					errno_exit("VIDIOC_QBUF");
	}
	type = buf_type;
	if (-1 == xioctl(fd, VIDIOC_STREAMON, &type))
			errno_exit("VIDIOC_STREAMON");

}

static void process_buffer(struct buffer* buff, int size)
{
#ifdef SAVE_IMAGE
	if (fp) {
		fseek(fp, 0L, SEEK_SET);
		fwrite(buff->start, size, 1, fp);
		fflush(fp);
	}
#endif

#ifdef HAVE_OPENCV
	cv::Mat yuvmat(cv::Size(width, height*3/2), CV_8UC1, buff->start);
	cv::Mat rgbmat(cv::Size(width, height), CV_8UC3);
	cv::cvtColor(yuvmat, rgbmat, CV_YUV2BGR_NV12);
	cv::imshow("video", rgbmat);
	cv::waitKey(1);
#endif
}


static int read_frame()
{
	struct v4l2_buffer buf;
	int i, bytesused;

	CLEAR(buf);

	buf.type = buf_type;
			buf.memory = V4L2_MEMORY_MMAP;

	if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == buf_type) {
			struct v4l2_plane planes[FMT_NUM_PLANES];
			buf.m.planes = planes;
			buf.length = FMT_NUM_PLANES;
	}

	if (-1 == xioctl(fd, VIDIOC_DQBUF, &buf))
			errno_exit("VIDIOC_DQBUF");

	i = buf.index;

	if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == buf_type)
			bytesused = buf.m.planes[0].bytesused;
	else
			bytesused = buf.bytesused;
			process_buffer(&(buffers[i]), bytesused);
	//DBG("bytesused %d\n", bytesused);

	if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
			errno_exit("VIDIOC_QBUF");

	return 1;
}

static unsigned long get_time(void)
{
	struct timeval ts;
	gettimeofday(&ts, NULL);
	return (ts.tv_sec * 1000 + ts.tv_usec / 1000);
}


void *thread_loop(void *param)
{
	unsigned int count = 1;
	unsigned long read_start_time, read_end_time;
#ifdef SAVE_IMAGE
	fp = fopen(dump_path, "wb");
#endif
	while (1) {
		read_start_time = get_time();
		read_frame();
		read_end_time = get_time();
		count++;

		DBG("FrameNr: %d\t", count);        //Display the current image frame number
		DBG("pull time %lu ms\n",read_end_time - read_start_time);
	}
	DBG("\nREAD AND SAVE DONE!\n");
	return NULL;
}

void print_usage(void)
{
    printf("vcap /dev/videoX\n");
}

int main(int argc, char *argv[])
{
    if (argc <= 1)
    {
        print_usage();
        return 0;
    }

    dump_path = "./vcap_dump.jpg";
    width =  640;
    height = 400;
    v4l2_buf_type buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    format = V4L2_PIX_FMT_MJPEG;

    if (strncmp(argv[1], "-d", 2) == 0) {
    	width =  640;
    	height = 400;
    	dump_path = "./vcap_dump.raw";
    	format = V4L2_PIX_FMT_YUYV;

    	dev_name = argv[2]; 
    } else {
    	dev_name = argv[1];
    }
 
    open_device();
    init_device();
    start_capturing();

/*
    static pthread_t thread_id;    
    pthread_create(&thread_id, NULL, thread_loop, NULL);

    while(1) {
    	sleep(1);
    }
*/
    thread_loop(NULL);

    return 0;
}
