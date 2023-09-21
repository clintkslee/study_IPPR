#pragma warning(disable:4996)
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <Math.h>

// Output 배열에 저장된 영상을 FileName 파일로 출력
void SaveBMPFile(BITMAPFILEHEADER hf, BITMAPINFOHEADER hInfo, RGBQUAD* hRGB, BYTE* Output, int W, int H, const char* FileName)
{
	FILE* fp = fopen(FileName, "wb");
	fwrite(&hf, sizeof(BYTE), sizeof(BITMAPFILEHEADER), fp);
	fwrite(&hInfo, sizeof(BYTE), sizeof(BITMAPINFOHEADER), fp);
	fwrite(hRGB, sizeof(RGBQUAD), 256, fp);
	fwrite(Output, sizeof(BYTE), W * H, fp);
	fclose(fp);
}

// 영상 반전
void InverseImage(BYTE* Img, BYTE *Out, int W, int H) // 원본영상, 출력영상, 가로길이, 세로길이
{
	int ImgSize = W * H;
	for (int i = 0; i < ImgSize; i++)
	{
		Out[i] = 255 - Img[i];
	}
}

// 영상의 화소값 히스토그램 
void ObtainHistogram(BYTE* Img, int* Histo, int W, int H)
{
	int ImgSize = W * H;
	for (int i = 0; i < ImgSize; i++) {
		Histo[Img[i]]++;
	}
}

// 영상의 화소값 누적히스토그램
void ObtainAHistogram(int* Histo, int* AHisto)
{
	for (int i = 0; i < 256; i++) {
		for (int j = 0; j <= i; j++) {
			AHisto[i] += Histo[j];
		}
	}
}

void ObtainAverageAndStdev(int* Histo, double* average, double* stdev)
{
	double Deviation[256] = { 0, }, sum3 = 0, variance;
	int sum1 = 0, sum2 = 0;
	for (int i = 0; i < 256; i++)
	{
		sum1 += i * Histo[i];		// (계급값 * 도수) 의 총합
		sum2 += Histo[i];			// (도수) 의 총합
	}
	//printf("sum1 is %d\nsum2 is %d\n", sum1, sum2);
	*average = (double)sum1 / sum2;	// 평균

	for (int i = 0; i < 256; i++)	// 편차^2 * 도수 합
	{
		Deviation[i] = (double)Histo[i] - *average;
		Deviation[i] = Deviation[i] * Deviation[i] * i;
		sum3 += Deviation[i];	
	}
	//printf("sum3 is %lf\n", sum3);
	variance = (double)sum3 / sum2;	// 분산 = (편차^2*도수) / (도수의 총합)
	*stdev = sqrt(variance);		// 표준편차 = sqrt(분산)
}

// 히스토그램 평활화
void HistogramEqualization(BYTE* Img, BYTE* Out, int* AHisto, int W, int H)
{
	int ImgSize = W * H;
	int Nt = W * H;	// 영상의 화소 개수
	int Gmax = 255; // 영상의 화소가 가질 수 있는 최대값
	double Ratio = Gmax / (double)Nt;
	BYTE NormSum[256]; 
	for (int i = 0; i < 256; i++) {
		NormSum[i] = (BYTE)(Ratio * AHisto[i]);	// 평활화 공식, 화소는 정수값만 취하므로 소수점 이하 버림
	}
	for (int i = 0; i < ImgSize; i++)
	{
		Out[i] = NormSum[Img[i]];
	}
}

// 애버리지 컨볼루션
void AverageConv(BYTE* Img, BYTE* Out, int W, int H)
{
	double Kernel[3][3] = { 0.11111, 0.11111, 0.11111,
							0.11111, 0.11111, 0.11111,
							0.11111, 0.11111, 0.11111 };
	double SumProduct = 0.0;					// 컨볼루션 계산 누적합 저장 변수

	for (int i = 1; i < H-1; i++)				// Y좌표(행), 마진 고려하여 1 감소
	{
		for (int j = 1; j < W-1; j++)			// X좌표(열)
		{
			for (int m = -1; m <= 1; m++)		// 커널의 행, [i][j] 중심으로 9칸 순회
			{
				for (int n = -1; n <= 1; n++)	// 커널의 열
				{
					SumProduct += Img[(i+m) * W + (j+n)] * Kernel[m+1][n+1];	// -1 ~ 1 범위 고려하여 +1

				}
			}
			Out[i * W + j] = (BYTE)SumProduct;
			SumProduct = 0.0;
		}
	}
}

int main()
{
	BITMAPFILEHEADER hf;		// 14바이트
	BITMAPINFOHEADER hInfo;		// 40바이트
	RGBQUAD hRGB[256];			// 1024바이트
	FILE* fp;
	fp = fopen("lenna.bmp", "rb");
	if (fp == NULL) {
		printf("File not found!\n");
		return -1;
	}

	fread(&hf, sizeof(BITMAPFILEHEADER), 1, fp);
	fread(&hInfo, sizeof(BITMAPINFOHEADER), 1, fp);
	fread(hRGB, sizeof(RGBQUAD), 256, fp);
	int ImgSize = hInfo.biWidth * hInfo.biHeight;
	BYTE * Image = (BYTE *)malloc(ImgSize);	// 원본 영상 배열
	BYTE* Temp = (BYTE*)malloc(ImgSize);	// 임시 영상 배열
	BYTE * Output = (BYTE*)malloc(ImgSize);	// 출력 영상 배열
	fread(Image, sizeof(BYTE), ImgSize, fp);
	fclose(fp);

	// 역상(inverse) 영상을 구하기
	InverseImage(Image, Temp, hInfo.biWidth, hInfo.biHeight);
	SaveBMPFile(hf, hInfo, hRGB, Temp, hInfo.biWidth, hInfo.biHeight, "lenna_inverse.bmp");	// 역상 영상 (결과 영상)

	// 1. 결과 영상에 대해 3*3 평균필터 컨볼루션을 수행
	AverageConv(Temp, Output, hInfo.biWidth, hInfo.biHeight);
	SaveBMPFile(hf, hInfo, hRGB, Output, hInfo.biWidth, hInfo.biHeight, "lenna_conv.bmp");	// 결과 영상에 대해 평균필터 컨볼루션 수행 후 영상

	// 2. 결과 영상에 대한 히스토그램 평활화를 수행
	// 3. 결과 영상의 평균밝기 및 표준편차를 계산
	int Histo[256] = {0, };																	// 결과 영상의 히스토그램 담을 배열
	ObtainHistogram(Temp, Histo, hInfo.biWidth, hInfo.biHeight);							// 평활화 전 히스토그램 구하기

	double average, stdev; 																	// 평활화 전 결과 영상의 평균 밝기 및 표준 편차 담을 변수
	ObtainAverageAndStdev(Histo, &average, &stdev);											// 평활화 전 결과 영상의 평균 밝기 및 표준 편차 계산

	FILE* fp2 = fopen("histo.txt", "w");													// 평활화 전 히스토그램, 평균 밝기, 표준 편차 출력 -> histo.txt
	for (int i = 0; i < 256; i++) fprintf(fp, "%d\t%d\n", i, Histo[i]);	
	fprintf(fp, "Average brightness : %lf\t\tStandard Deviation : %lf\n", average, stdev);
	fclose(fp2);

	int AHisto[256] = {0, };																// 결과 영상의 누적히스토그램 담을 배열
	ObtainAHistogram(Histo, AHisto);														// 누적 히스토그램 구하기

	HistogramEqualization(Temp, Output, AHisto, hInfo.biWidth, hInfo.biHeight);				// 히스토그램 평활화
	SaveBMPFile(hf, hInfo, hRGB, Output, hInfo.biWidth, hInfo.biHeight, "lenna_heq.bmp");	// 결과 영상에 대해 평활화 수행 후 영상

	int Histo2[256] = { 0, };																// 펼활화 후 히스토그램 담을 배열
	ObtainHistogram(Output, Histo2, hInfo.biWidth, hInfo.biHeight);							// 평활화 후 히스토그램 구하기
	double average2, stdev2;																// 평활화 후 히스토그램에 대해 평균 밝기 및 표준 편차 담을 변수
	ObtainAverageAndStdev(Histo2, &average2, &stdev2);										// 평활화 후 영상의 평균 밝기 및 표준 편차 계산

	fp2 = fopen("histo_equ.txt", "w");														// 평활화 후 히스토그램, 평균 밝기, 표준 편차 출력 -> histro_equ.txt
	for (int i = 0; i < 256; i++) fprintf(fp, "%d\t\t%d\n", i, Histo[i]);
	fprintf(fp, "Average brightness : %lf\tStandard Deviation : %lf\n", average2, stdev2);
	fclose(fp2);
	
	

	
	



	free(Image);
	free(Temp);
	free(Output);
	return 0;
}