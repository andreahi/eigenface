#include <iostream>
#include <Eigen/Dense>
#include <Eigen/Eigenvalues> 
#include "Source.h"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <CL/cl.h>
#include <time.h>       /* time */
#include <chrono>
#include <Windows.h>
#define MEM_SIZE (128)
#define MAX_SOURCE_SIZE (0x100000)

#define CL_CHECK(_expr)                                                         \
   do {                                                                         \
     cl_int _err = _expr;                                                       \
     if (_err == CL_SUCCESS)                                                    \
       break;                                                                   \
     fprintf(stderr, "OpenCL Error: '%s' returned %d!\n", #_expr, (int)_err);   \
     abort();                                                                   \
         } while (0)

#define CL_CHECK_ERR(_expr)                                                     \
   ({                                                                           \
     cl_int _err = CL_INVALID_VALUE;                                            \
     typeof(_expr) _ret = _expr;                                                \
     if (_err != CL_SUCCESS) {                                                  \
       fprintf(stderr, "OpenCL Error: '%s' returned %d!\n", #_expr, (int)_err); \
       abort();                                                                 \
	 	      }                                                                     \
     _ret;                                                                      \
	        })
using namespace cv;

using Eigen::MatrixXd;
using namespace std;
using namespace Eigen;


const float IMAGE_SCALE = 0.5;
const int NR_IMAGES = 30;
const int NR_FEATURES = 30;

double min_m(MatrixXf data, int size){
	double val = data(0);
	for (int i = 0; i < size; i++){
		if (val < data(i))
			val = data(i);
	}
	return val;
}

double max_m(MatrixXf data, int size){
	double val = data(0);
	for (int i = 0; i < size; i++){
		if (val > data(i))
			val = data(i);
	}
	return val;
}

int get_data(MatrixXf *Img_test, MatrixXf *A){

	int imageW1 = 250 * IMAGE_SCALE, imageH1 = 250 * IMAGE_SCALE;

	 *Img_test = MatrixXf::Ones(imageH1 * imageH1, 1);
	 *A = MatrixXf::Random(imageW1*imageH1, NR_IMAGES);

	Mat src, dst;

	char* source_window = "Source image";
	char* equalized_window = "Equalized Image";
	char* scaled_window = "scaled Image";

	/// Load image
	src = imread("images\\0.bmp", 1);

	if (!src.data)
	{
		cout << "Usage: ./Histogram_Demo <path_to_image>" << endl;
		return -1;
	}
	if (src.isContinuous())
		cout << "continuous" << endl;
	/// Convert to grayscale
	cvtColor(src, src, CV_BGR2GRAY);

	/// Apply Histogram Equalization
	equalizeHist(src, dst);


	/// Display results
	namedWindow(equalized_window, CV_WINDOW_AUTOSIZE);
	namedWindow(equalized_window, CV_WINDOW_AUTOSIZE);

	Mat scale_img;
	resize(src, scale_img, Size(), 0.12649, 0.12649);
	Mat scale_i;
	resize(scale_img, scale_i, Size(), 4, 4);
	namedWindow(scaled_window, CV_WINDOW_AUTOSIZE);
	imshow(scaled_window, scale_i);

	imshow(source_window, src);

	resize(src, dst, Size(), IMAGE_SCALE, IMAGE_SCALE);


	imshow(equalized_window, dst);

	/// Wait until user exits the program
	//waitKey(0);


	
	uchar4 **h_Src = (uchar4 **)malloc(NR_IMAGES*sizeof(uchar4*));


	for (int i = 0; i < NR_IMAGES; i++){
		char image_path[50];
		sprintf_s(image_path, "images\\i (%d).bmp", i + 1000);//sdkFindFilePath("portrait_noise.bmp", argv[0]);
		//printf("%s\n", image_path);

		LoadBMPFile(&h_Src[i], &imageW1, &imageH1, image_path);

		src = imread(image_path, 1);
		resize(src, dst, Size(), IMAGE_SCALE, IMAGE_SCALE);
		resize(src, src, Size(), IMAGE_SCALE, IMAGE_SCALE);

		imageH1 = dst.rows;
		imageW1 = dst.cols;
		for (int j = 0; j < imageW1 * imageH1; j += 1){

			h_Src[i][j].x = dst.data[3 * j + 0];
			h_Src[i][j].y = dst.data[3 * j + 1];
			h_Src[i][j].z = dst.data[3 * j + 2];
		}

	}


	char image_path[50];
	sprintf_s(image_path, "images\\%d.bmp", 0);//sdkFindFilePath("portrait_noise.bmp", argv[0]);
	printf("%s\n", image_path);


	src = imread(image_path, 1);
	resize(src, dst, Size(), IMAGE_SCALE, IMAGE_SCALE);
	resize(src, src, Size(), IMAGE_SCALE, IMAGE_SCALE);

	for (int j = 0; j < imageW1 * imageH1; j += 1){
		(*Img_test)(j, 0) = (dst.data[3 * j + 0] + dst.data[3 * j + 1] + dst.data[3 * j + 2]) / 3;
	}


	//	std::cin.get();
	//	SaveBMPFile(h_Src[0], &imageW1, &imageH1, "0.bmp", "test.bmp");


	cout << "imageW1: " << imageW1 << " imageH1: " << imageH1 << endl;
	EigenSolver<MatrixXf> es;
	for (int i = 0; i < NR_IMAGES; i++){
		for (int j = 0; j < imageW1 * imageH1; j++){
			(*A)(j, i) = (h_Src[i][j].x + h_Src[i][j].y + h_Src[i][j].z) / 3.0;
			//	cout << "x: " << h_Src[i][j].x;
		}
	}




}

template<typename TimeT = std::chrono::milliseconds>
struct measure
{
	template<typename F, typename ...Args>
	static typename TimeT::rep execution(F func, Args&&... args)
	{
		auto start = std::chrono::system_clock::now();
		func(std::forward<Args>(args)...);
		auto duration = std::chrono::duration_cast< TimeT>
			(std::chrono::system_clock::now() - start);
		return duration.count();
	}
};

int min_cpu(int *values, int size){
	int smallest = values[0];
	for (int i = 0; i < size; i++){
		if (values[i] < smallest){
			smallest = values[i];
		}
	}
	return smallest;
}
int min_cpu_i(int *values, int size){
	int smallest = values[0];
	int index = 0;
	for (int i = 0; i < size; i++){
		if (values[i] < smallest){
			smallest = values[i];
			index = i;
		}
	}
	return index;
}
int GPU_min(){



	

	cl_device_id device_id = NULL;
	cl_context context = NULL;
	cl_command_queue command_queue = NULL;
	cl_mem memobj = NULL;
	cl_mem numbers = NULL;
	cl_mem out_numbers = NULL;
	cl_mem size_d = NULL;

	cl_program program = NULL;
	cl_kernel kernel = NULL;
	cl_platform_id platform_id = NULL;
	cl_uint ret_num_devices;
	cl_uint ret_num_platforms;
	cl_int ret;

	cl_platform_id platforms[100];
	cl_uint platforms_n = 0;
	CL_CHECK(clGetPlatformIDs(100, platforms, &platforms_n));

	printf("=== %d OpenCL platform(s) found: ===\n", platforms_n);
	for (int i = 0; i < platforms_n; i++)
	{
		char buffer[10240];
		printf("  -- %d --\n", i);
		CL_CHECK(clGetPlatformInfo(platforms[i], CL_PLATFORM_PROFILE, 10240, buffer, NULL));
		printf("  PROFILE = %s\n", buffer);
		CL_CHECK(clGetPlatformInfo(platforms[i], CL_PLATFORM_VERSION, 10240, buffer, NULL));
		printf("  VERSION = %s\n", buffer);
		CL_CHECK(clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, 10240, buffer, NULL));
		printf("  NAME = %s\n", buffer);
		CL_CHECK(clGetPlatformInfo(platforms[i], CL_PLATFORM_VENDOR, 10240, buffer, NULL));
		printf("  VENDOR = %s\n", buffer);
		CL_CHECK(clGetPlatformInfo(platforms[i], CL_PLATFORM_EXTENSIONS, 10240, buffer, NULL));
		printf("  EXTENSIONS = %s\n", buffer);
	}



	char string[MEM_SIZE];

	FILE *fp;
	char fileName[] = "./kernel.cl";
	char *source_str;
	size_t source_size;

	cl_int clerr = CL_SUCCESS;

	/* Load the source code containing the kernel*/
	fopen_s(&fp, fileName, "r");
	if (!fp) {
		fprintf(stderr, "Failed to load kernel.\n");
		exit(1);
	}
	source_str = (char*)malloc(MAX_SOURCE_SIZE);
	source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);
	fclose(fp);

	/* Get Platform and Device Info */
	ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);
	ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_DEFAULT, 1, &device_id, &ret_num_devices);
	char* vendor_name = (char*)malloc(1000 * sizeof(char));
	size_t return_size;
	//clGetDeviceInfo(device_id, CL_DEVICE_PROFILE, sizeof(vendor_name), vendor_name, &return_size);
	//clGetDeviceInfo(device_id, CL_DEVICE_NAME, sizeof(vendor_name), vendor_name, NULL);
	//puts(vendor_name);
	//printf("size: %d\n", return_size);
	//printf("name: %s\n", vendor_name);
	/* Create OpenCL context */
	context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &ret);

	/* Create Command Queue */
	command_queue = clCreateCommandQueue(context, device_id, 0, &ret);

	const int  NUMBER_SIZE = 1<<15;
	//int * h_number = (int*)malloc(10 * sizeof(int));
	int * out_number_h = (int*)malloc(NUMBER_SIZE * sizeof(int));
	int *h_number = (int*)malloc(NUMBER_SIZE * sizeof(int));
	srand(time(NULL));
	for (int i = 0; i < NUMBER_SIZE; i++)
		h_number[i] = rand() % 10000 + rand() % 10000;
	

	/* Create Memory Buffer */
	memobj = clCreateBuffer(context, CL_MEM_READ_WRITE, MEM_SIZE * sizeof(char), NULL, &ret);
	if (clerr != CL_SUCCESS)
		printf("error: clCreateBuffer\n");
	numbers = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, NUMBER_SIZE * sizeof(int), h_number, &ret);
	if (clerr != CL_SUCCESS)
		printf("error: clCreateBuffer\n");
	out_numbers = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, NUMBER_SIZE * sizeof(int), h_number, &ret);
	if (clerr != CL_SUCCESS)
		printf("error: clCreateBuffer\n");



	/* Create Kernel Program from the source */
	program = clCreateProgramWithSource(context, 1, (const char **)&source_str,
		(const size_t *)&source_size, &clerr);

	if (clerr != CL_SUCCESS)
		printf("error: clCreateProgramWithSource\n");
	/* Build Kernel Program */
	ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);

	/* Create  Kernel from function with name in second argument */
	kernel = clCreateKernel(program, "min_kernel", &ret);

	/* Set Kernel Parameters */
	ret = clSetKernelArg(kernel, 0, sizeof(numbers), NULL);
	ret = clSetKernelArg(kernel, 1, sizeof(numbers), (void *)&numbers);
	ret = clSetKernelArg(kernel, 2, sizeof(out_numbers), (void *)&out_numbers);
	ret = clSetKernelArg(kernel, 3, sizeof(NUMBER_SIZE), (void *)&NUMBER_SIZE);
	//ret = clSetKernelArg(kernel, 4, sizeof(NUMBER_SIZE), (void *)&NUMBER_SIZE);


	size_t global_item_size = 1;
	size_t local_work_size[] = { 2 };
	size_t global_work_size[] = {NUMBER_SIZE};
	cl_event kernel_completion;

	/* Enqueue the kernel with a given number of threads */
	  CL_CHECK(clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, global_work_size, NULL, 0, NULL, &kernel_completion));
	
	CL_CHECK(clWaitForEvents(1, &kernel_completion));
	CL_CHECK(clReleaseEvent(kernel_completion));
	if (clerr != CL_SUCCESS)
		printf("error: clEnqueueNDRangeKernel\n");

	//ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL,
	//	&global_item_size, &local_item_size, 0, NULL, NULL);

	/* Execute OpenCL Kernel */
	//ret = clEnqueueTask(command_queue, kernel, 0, NULL, NULL);

	/* Copy results from the memory buffer */
	ret = clEnqueueReadBuffer(command_queue, memobj, CL_TRUE, 0,
		MEM_SIZE * sizeof(char), string, 0, NULL, NULL);

	ret = clEnqueueReadBuffer(command_queue, out_numbers, CL_TRUE, 0,
		NUMBER_SIZE * sizeof(int), out_number_h, 0, NULL, NULL);
	/* Display Result */
	puts(string);
	long int before = GetTickCount();


	int smallest= min_cpu(h_number, NUMBER_SIZE);
	long int after = GetTickCount();

	cout << "time: " << after - before << endl;
	cout << "Smallest number found by GPU: " << out_number_h[0] << endl;
	
	cout << "Smallest number found by CPU: " << smallest << " with index: " << min_cpu_i(h_number, NUMBER_SIZE) << endl;

	//for (int i = 0; i < NUMBER_SIZE; i++)
	//	cout << out_number_h[i] << ", ";
	cout << endl;

	int tall = 3;
	cout << "tall: " << tall / 2 + ((tall / 2) & 1) << endl;
	/* Finalization */
	ret = clFlush(command_queue);
	ret = clFinish(command_queue);
	ret = clReleaseKernel(kernel);
	ret = clReleaseProgram(program);
	ret = clReleaseMemObject(memobj);
	ret = clReleaseCommandQueue(command_queue);
	ret = clReleaseContext(context);

	free(source_str);

	return 0;
}

int main()
{
	//int a = 0;
	//GPU_min();
	//cin >>a;

	///return 0;
	Mat src, dst;

	
	int imageW1 = 125, imageH1 = 125;

	/*A << 3, 3, 3, 3, 4,
		2, 2, 2, 2, 3,
		2, 2, 2, 2, 3,
		2, 2, 2, 2, 3,
		2, 2, 2, 2, 3,
		2, 2, 2, 2, 3,
		2, 2, 5, 2, 3,
		2, 2, 2, 2, 3,
		2, 2, 2, 2, 3,
		2, 2, 2, 2, 5;
	*/
	MatrixXf Img_test, A;


	get_data(&Img_test, &A);

	//cout << "A" << endl << A << endl;
	cout << "calculating" << endl;
	//es.compute(A, /* computeEigenvectors = */ true);
	//cout << "matrix:" << endl << A << endl;
//	cout << "The eigenvalues of A are: " << es.eigenvalues().transpose() << endl;
//	cout << "The eigenvectors of A are: " << es.eigenvectors().transpose() << endl;
	//MatrixXf mea = A.rowwise() - A.rowwise().mean();
	MatrixXf mu = A.rowwise().mean();

	A = A.transpose().eval();
	//MatrixXf centered = A.rowwise() - A.colwise().mean();
	//MatrixXf cov = centered.adjoint() * centered;

	A = A.colwise() - A.rowwise().mean();
	/*
	SelfAdjointEigenSolver<MatrixXf> eig(cov);
	MatrixXf W = eig.eigenvectors().rightCols(nr_features);
	*/
	cout << "calculating e" << endl;

	MatrixXf cov_e = A*A.transpose();
	SelfAdjointEigenSolver<MatrixXf> eig_e(cov_e);
	MatrixXf W_e = eig_e.eigenvectors().rightCols(NR_FEATURES);
	//W_e.normalize();
	MatrixXf v_i = A.transpose() * W_e;
	for (int i = 0; i < v_i.cols(); i++)
		v_i.col(i).normalize();

	src = imread("images\\0.bmp", 1);

	resize(src, dst, Size(), IMAGE_SCALE, IMAGE_SCALE);
	resize(src, src, Size(), IMAGE_SCALE, IMAGE_SCALE);
	

	float max_val = 0;

	for (int i = 0; i < NR_IMAGES; i++){
		MatrixXf y;
		//y = W.transpose() *A.row(i).transpose();
		y = v_i.transpose() *A.row(i).transpose();

		//cout << i <<"component: " << endl << y << endl;
	//	cout << "restored: " << endl << W *y + mu << endl;
		if (i == 1 || i == 2 || i == 4 || i == 5){
			MatrixXf restored;
			//restored = W *y + mu;
			restored = v_i *y + mu;
			cout << "rows " << W_e.cols() << endl;

			double min_val = min_m(restored, imageH1*imageW1);
			double max_val = max_m(restored, imageH1*imageW1);
			int num_bins = 256;
			for (int j = 0; j < imageH1*imageW1; j++){
				
				//if (restored(j) > 255)
				//	restored(j) = 255;
				
				//	restored(j) = 255;
				float bin = min((int)((restored(j) - min_val) / (max_val - min_val) * num_bins), num_bins - 1);
				bin = restored(j);
				
				src.data[3 * j] = bin;
				src.data[3 * j + 1] = bin;
				src.data[3 * j + 2] = bin;// restored(j);

				if (max_val < restored(j))
					max_val = restored(j);
				//if (restored(j) < 0)
				//	restored(j) = 0;
	

			}
	

			char equalized_window[50];
			sprintf_s(equalized_window, "i (%d).bmp", i + 1);//sdkFindFilePath("portrait_noise.bmp", argv[0]);

			/// Display results
			namedWindow(equalized_window, CV_WINDOW_AUTOSIZE);

			imshow(equalized_window, src);
			cout << "size: " << src.cols << endl;

			//char image_path[50];
			//sprintf_s(image_path, "o(%d).bmp", i);//sdkFindFilePath("portrait_noise.bmp", argv[0]);
			//SaveBMPFile(h_Src[0], &imageW1, &imageH1, "0.bmp", image_path);
			

		}

	}
	cout << "max val: " << max_val << endl;



	{
		MatrixXf y;
		//y = W.transpose() *A.row(i).transpose();
		y = Img_test;
		y -= mu;
		y=y.transpose() * v_i;
		//y = v_i.transpose() *(Img_test-mu);

		//cout << i <<"component: " << endl << y << endl;
		//	cout << "restored: " << endl << W *y + mu << endl;
			MatrixXf restored;
			//restored = W *y + mu;
			restored = v_i *y.transpose() + mu;
			

			cout << "rows " << W_e.cols() << endl;
			double min_val = min_m(restored, imageH1*imageW1);
			double max_val = max_m(restored, imageH1*imageW1);
			int num_bins = 256;
			for (int j = 0; j < imageH1*imageW1; j++){

				//if (restored(j) > 255)
				//	restored(j) = 255;
				int bin = min((int)((restored(j) - min_val) / (max_val - min_val) * num_bins), num_bins - 1);
				bin = 255 - bin;
				
				src.data[3 * j] = bin;
				src.data[3 * j + 1] = bin;
				src.data[3 * j + 2] = bin;// restored(j);
				if (max_val > restored(j))
					max_val = restored(j);
				//if (restored(j) < 0)
				//	restored(j) = 0;
			}


			char equalized_window[50];
			sprintf_s(equalized_window, "i (%d).bmp", 0);//sdkFindFilePath("portrait_noise.bmp", argv[0]);

			/// Display results
			namedWindow(equalized_window, CV_WINDOW_AUTOSIZE);

			imshow(equalized_window, src);
			cout << "size: " << src.cols << endl;

		
	}

	cout << "DONE..." << endl;
	waitKey(0);
}