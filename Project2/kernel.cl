__kernel void min_kernel(__local int s, __global int* numbers, __global int* out,  int size)
{
	//if (get_global_id(0) == 0){
	//string[get_global_id(0)] = numbers[get_global_id(0)] + 'A';
	int tid = get_global_id(0);
	for (int i = (size) / 2; i > 0; i = i / 2){
		if (numbers[tid] > numbers[tid + i] && i > tid)
			numbers[tid] = numbers[tid + i];
		barrier(CLK_LOCAL_MEM_FENCE | CLK_GLOBAL_MEM_FENCE);
	}
	barrier(CLK_LOCAL_MEM_FENCE | CLK_GLOBAL_MEM_FENCE);
	if (tid == 0)
		out[tid] = numbers[tid] < numbers[tid+1] ? numbers[tid] : numbers[tid+1];
	else
	
	out[tid] = get_local_size(0);
	//out[tid] = tid;
}

__kernel void gf(__global char* string, __global int* numbers)
{
	//if (get_global_id(0) == 0){
	string[get_global_id(0)] = numbers[get_global_id(0)] + 'A';
	/*string[1] = 'f';
	string[2] = 'l';
	string[3] = 'l';
	string[4] = 'o';
	string[5] = ',';
	string[6] = ' ';
	string[7] = 'W';
	string[8] = 'o';
	string[9] = 'r';
	string[10] = 'l';
	string[11] = 'd';
	string[12] = '!';
	string[13] = '\0';*/
	//}
}