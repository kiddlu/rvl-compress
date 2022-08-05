#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <pthread.h>

#define CAMERA_INTRINSIC_PARAMS_LEN       (36)
#define ROTATE_INTRINSIC_PARAMS_LEN       (36)
#define TRANSLATION_INTRINSIC_PARAMS_LEN  (12)

typedef struct
{
        uint8_t   color_sensor_intrinsic_params[CAMERA_INTRINSIC_PARAMS_LEN];     //36 bytes 9 float
        uint8_t   irIntrinsicParams[CAMERA_INTRINSIC_PARAMS_LEN];                 //36 bytes 9 float
        uint8_t   irhd_sensor_intrinsic_params[CAMERA_INTRINSIC_PARAMS_LEN];      //36 bytes 9 float
        uint8_t   rotate_intrinsic_params[ROTATE_INTRINSIC_PARAMS_LEN];           //36 bytes 9 float
        uint8_t   translation_intrinsic_params[TRANSLATION_INTRINSIC_PARAMS_LEN]; //12 bytes 3 float
        uint8_t   reserv[100];
}__attribute__((packed)) camera_params_t;

typedef struct
{
	uint8_t   init;

	float     color_intri[9];
	float     irhd_intri[9];
	float     trans_intri[3];
	float     rot_intri[9];
	
	//special settings
	uint32_t zshift_level;
}registration_params_t;

static registration_params_t rparams;

void algo_registration_params_init(int width , int height, camera_params_t *cprams)
{
	if (rparams.init == 1) {
		return;
	}

	printf("registration init params width = %d, height = %d\n", width, height);

	//camera_params_t *cprams = (camera_params_t *)os_devparams_get()->intrinsic_params;

	memcpy(rparams.color_intri, cprams->color_sensor_intrinsic_params, sizeof(cprams->color_sensor_intrinsic_params));
	memcpy(rparams.irhd_intri,  cprams->irhd_sensor_intrinsic_params,  sizeof(cprams->irhd_sensor_intrinsic_params));
	memcpy(rparams.rot_intri,   cprams->rotate_intrinsic_params,       sizeof(cprams->rotate_intrinsic_params));
	memcpy(rparams.trans_intri, cprams->translation_intrinsic_params,  sizeof(cprams->translation_intrinsic_params));

	if (width <= 320) {
		for(int i = 0 ; i< 9; i++) {
			rparams.color_intri[i] /= 4.0;
			rparams.irhd_intri[i] /= 4.0;
		}
	} else if(width <= 640) {
		for(int i = 0 ; i< 9; i++) {
			rparams.color_intri[i] /= 2.0;
			rparams.irhd_intri[i] /= 2.0;
		}
	}

	rparams.zshift_level = 4;

	rparams.init = 1;

	return;
}

void algo_registration_params_clean(void)
{
	if (rparams.init == 0) {
		return;
	}

	memset(&rparams, 0, sizeof(rparams));

	rparams.init = 0;

	return;
}

void algo_registration_convert(uint16_t  *src_img, uint16_t  *dst_img, uint32_t src_width, uint32_t src_height)
{

	if(rparams.init == 0) {
		return;
	}

	float xdepth, ydepth, zdepth, xturn, yturn, zturn;
	int xcam, ycam;

	int widthlim     = src_width;
	int widthlim_dst = src_width;
	int heightlim    = src_height - 1;

	int start_y =  0;           //startat[thindex];
	int end_y   =  src_height;   //widthlim * heightlim;//startat[thindex + 1];

	int src_index = 0;
	int dst_index = 0; 

	int cur_y = 0;

    struct timespec tstart, tend;
    uint32_t diff = 0;

	for(cur_y = start_y ; cur_y < end_y; cur_y++)
	{
		if(cur_y >= src_height) {
			return;
		}

		for (int cur_x = 0; cur_x < src_width; cur_x++)
		{
			src_index = cur_y *src_width + cur_x;

	
			uint16_t src_dataval = src_img[src_index];

			if(src_dataval == 0) {
				continue;
			}

			zdepth = src_dataval >> rparams.zshift_level;

			if (zdepth <= 100) {
				continue;
			}

			xdepth = ((float)cur_x - rparams.irhd_intri[2]) * zdepth / rparams.irhd_intri[0];								//深度图转点云
			ydepth = ((float)cur_y - rparams.irhd_intri[3]) * zdepth / rparams.irhd_intri[1];


			xturn = xdepth * rparams.rot_intri[0] + ydepth * rparams.rot_intri[1] + zdepth * rparams.rot_intri[2] + rparams.trans_intri[0];				//点云坐标变换
			yturn = xdepth * rparams.rot_intri[3] + ydepth * rparams.rot_intri[4] + zdepth * rparams.rot_intri[5] + rparams.trans_intri[1];
			zturn = xdepth * rparams.rot_intri[6] + ydepth * rparams.rot_intri[7] + zdepth * rparams.rot_intri[8] + rparams.trans_intri[2];

			xcam = rparams.color_intri[0] * xturn / zturn + rparams.color_intri[2];									//通过相机外参计算配准后点的位置
			ycam = rparams.color_intri[1] * yturn / zturn + rparams.color_intri[3];

			if (xcam < 0 || xcam >= widthlim_dst || ycam < 0 || ycam >= heightlim)
			{
				continue;
			}

			dst_index = xcam + ycam * widthlim_dst;

			dst_img[dst_index] = src_dataval;

            //printf("src_x %03d, src_y %03d, dst_x %03d, dst_y %03d\n", cur_x, cur_y, xcam, ycam);
		}

	}
    printf("Rundiff  %u us\n", diff / 1000);
    
}


int main()
{
	FILE *fp = fopen("intrinsic_params.bin", "r+b");
	char *inpams = malloc(256);
	fread(inpams, 256, 1, fp);
	fclose(fp);

	fp = fopen("src.depth", "r+b");
	char *src_img = malloc(640*400*2);
	fread(src_img, 640*400*2, 1, fp);
	fclose(fp);

	char *dst_img = malloc(640*400*2);


	algo_registration_params_init(640, 400, (camera_params_t *)inpams);

    struct timespec tstart, tend;
    uint32_t diff;

    clock_gettime(CLOCK_REALTIME, &tstart);
	algo_registration_convert(src_img, dst_img, 640, 400);
    clock_gettime(CLOCK_REALTIME, &tend);
    diff = (tend.tv_sec - tstart.tv_sec) * 1000000 + (tend.tv_nsec - tstart.tv_nsec) / 1000;
    printf("Rundiff  %5u us\n", diff);


	fp = fopen("dst.depth", "w+b");
	fwrite(dst_img, 640*400*2, 1, fp);
	fclose(fp);
}
