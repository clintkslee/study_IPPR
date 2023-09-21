#pragma warning(disable:4996)
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>

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

// 3주차
// 영상 반전
void InverseImage(BYTE* Img, BYTE *Out, int W, int H) // 원본영상, 출력영상, 가로길이, 세로길이
{
	int ImgSize = W * H;
	for (int i = 0; i < ImgSize; i++)
	{
		Out[i] = 255 - Img[i];
	}
}

// 영상 밝기 조절
void BrightnessAdj(BYTE* Img, BYTE* Out, int W, int H, int Val) // 원본영상, 출력영상, 가로길이, 세로길이, 조절값
{
	int ImgSize = W * H;
	for (int i = 0; i < ImgSize; i++)
	{
		if (Img[i] + Val > 255)	// 클리핑 처리 - 오버플로우 시 255
		{
			Out[i] = 255;
		}
		else if (Img[i] + Val < 0) // 클리핑 처리 - 언더플로우 시 0
		{
			Out[i] = 0;
		}
		else 	Out[i] =Img[i] + Val;
	}
}

// 영상 대비 조절
void ContrastAdj(BYTE* Img, BYTE* Out, int W, int H, double Val) // 원본영상, 출력영상, 가로길이, 세로길이, 조절값
{
	int ImgSize = W * H;
	for (int i = 0; i < ImgSize; i++)
	{
		if (Img[i] * Val > 255.0)	// Val 에 음수가 들어오는 경우는 없음 (화소값 음수 x)
		{
			Out[i] = 255;
		}
		else
		{
			Out[i] = (BYTE)(Img[i] * Val);
		}
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

// 히스토그램 스트레칭 
void HistogramStretching(BYTE * Img, BYTE * Out, int * Histo, int W, int H)
{
	int ImgSize = W * H;
	BYTE Low, High;
	for (int i = 0; i < 256; i++) {	// 히스토그램에서 처음으로 0개가 아닌 화소값 만나면 종료 (영상에서 가장 어두운 화소값)
		if (Histo[i] != 0) {
			Low = i;
			break;
		}
	}
	for (int i = 255; i >= 0; i--) { // 히스토그램에서 처음으로 0개가 아닌 화소값 만나면 종료 (영상에서 가장 밝은 화소값)
		if (Histo[i] != 0) {
			High = i;
			break;
		}
	}
	for (int i = 0; i < ImgSize; i++) {
		Out[i] = (BYTE)((Img[i] - Low) / (double)(High - Low) * 255.0); // 스트레칭 공식
	}
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

// 이진화 임계값 설정 (Gonzalez, Woods 경계값 자동 결정 방법)
int GozalezBinThresh(int* Histo)
{
	int T; // 임계값 
	int newT; // 반복문 1회 수행 후 계산되는 새로운 임계값
	int deltaT; // 임계값의 변화 (입실론과 비교)
	BYTE High, Low; 
	BYTE ep = 3; // 입실론 : 오차
	int G1, cnt1, mu1,	// 임계값 T 초과의 화소값들의 누적합, 개수, 평균
		G2, cnt2, mu2;	// 임계값 T 이하의 화소값들의 누적합, 개수, 평균
 
	for (int i = 255; i >= 0; i--) {
		if (Histo[i] != 0) {
			High = i;
			break;
		}
	}

	for (int i = 0; i < 256; i++) {	
		if (Histo[i] != 0) {
			Low = i;
			break;
		}
	}

	// 1. 초기 임계값 지정 : 영상 밝기 최대값과 최소값의 중간
	T = (High + Low) / 2;

	while (1)
	{
		G1 = 0; cnt1 = 0; mu1 = 0;
		G2 = 0; cnt2 = 0; mu2 = 0;

		// 2. 임계값 T로 이진화 (밝기값이 T보다 큰 화소들로 구성된 영역(G1)과 밝기값이 T 보다 작은 화소들로 구성된 영역(G2)으로 분할)
		for (int i = 255; i > T; i--)
		{
			if (Histo[i] != 0) {
				G1 += Histo[i] * i;	
				cnt1 += Histo[i];
			}
		}
		//printf("G1 = %d , cnt1 = %d\n", G1, cnt1);

		for (int i = 0; i <= T; i++)
		{
			if (Histo[i] != 0) {
				G2 += Histo[i] * i;
				cnt2 += Histo[i];
			}
		}
		//printf("G2 = %d , cnt2 = %d\n", G2, cnt2);

		// 3. 영역 G1과 G2에 대하여 화소들의 밝기의 평균값(mu1, mu2)을 계산
		mu1 = G1 / cnt1;
		mu2 = G2 / cnt2;

		// 4. 새로운 임계값 newT 계산
		newT = (mu1 + mu2) / 2;

		// 5. 연속적인 반복에서 경계 값의 변화가 미리 정의된 오차 입실론(ep)보다 작을 때까지 2~4단계 반복
		if (newT > T)	deltaT = newT - T;
		else deltaT = T - newT;
		T = newT;

		if (deltaT < ep) break;
		//else printf("T = %d\n", T);
	}
	//printf("T = %d\n", T);
	return T;
}

// 영상 이진화
void Binarization(BYTE * Img, BYTE * Out, int W, int H, BYTE Threshold)
{
	int ImgSize = W * H;
	for (int i = 0; i < ImgSize; i++) {
		if (Img[i] < Threshold) Out[i] = 0;
		else Out[i] = 255;
	}
}

// 4주차
// 애버리지 컨볼루션 (박스 평활화) -> 영상 블러링
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

// 가우시안 컨볼루션 (가우시안 평활화) -> 영상 블러링, 박스 평활화보다 원본 영상의 정보를 더 많이 갖는다
void GaussAvrConv(BYTE* Img, BYTE* Out, int W, int H)
{
	double Kernel[3][3] = { 0.0625, 0.125, 0.0625,
							0.125, 0.25, 0.125,
							0.0625, 0.125, 0.0625 };
	double SumProduct = 0.0;					// 컨볼루션 계산 누적합 저장 변수

	for (int i = 1; i < H - 1; i++)				// Y좌표(행), 마진 고려하여 1 감소
	{
		for (int j = 1; j < W - 1; j++)			// X좌표(열)
		{
			for (int m = -1; m <= 1; m++)		// 커널의 행, [i][j] 중심으로 9칸 순회
			{
				for (int n = -1; n <= 1; n++)	// 커널의 열
				{
					SumProduct += Img[(i + m) * W + (j + n)] * Kernel[m + 1][n + 1];	// -1 ~ 1 범위 고려하여 +1

				}
			}
			Out[i * W + j] = (BYTE)SumProduct;
			SumProduct = 0.0;
		}
	}
}

// Prewitt 마스크 X : X 방향으로 화소값 변화 지점 검출
void Prewitt_X_Conv(BYTE* Img, BYTE* Out, int W, int H)
{
	double Kernel[3][3] = { -1.0, 0.0, 1.0,
							-1.0, 0.0, 1.0,
							-1.0, 0.0, 1.0 };
	double SumProduct = 0.0;					// 컨볼루션 계산 누적합 저장 변수

	for (int i = 1; i < H - 1; i++)				// Y좌표(행), 마진 고려하여 1 감소
	{
		for (int j = 1; j < W - 1; j++)			// X좌표(열)
		{
			for (int m = -1; m <= 1; m++)		// 커널의 행, [i][j] 중심으로 9칸 순회
			{
				for (int n = -1; n <= 1; n++)	// 커널의 열
				{
					SumProduct += Img[(i + m) * W + (j + n)] * Kernel[m + 1][n + 1];	// -1 ~ 1 범위 고려하여 +1

				}
			}
			Out[i * W + j] = abs((long)SumProduct) / 3;	// normalize : 0 ~ 765 범위 -> 0 ~ 255
			SumProduct = 0.0;
		}
	}
}

// Prewitt 마스크 Y : Y 방향으로 화소값 변화 지점 검출
void Prewitt_Y_Conv(BYTE* Img, BYTE* Out, int W, int H)
{
	double Kernel[3][3] = { -1.0, -1.0, -1.0,
							0.0, 0.0, 0.0,
							1.0, 1.0, 1.0 };
	double SumProduct = 0.0;					// 컨볼루션 계산 누적합 저장 변수

	for (int i = 1; i < H - 1; i++)				// Y좌표(행), 마진 고려하여 1 감소
	{
		for (int j = 1; j < W - 1; j++)			// X좌표(열)
		{
			for (int m = -1; m <= 1; m++)		// 커널의 행, [i][j] 중심으로 9칸 순회
			{
				for (int n = -1; n <= 1; n++)	// 커널의 열
				{
					SumProduct += Img[(i + m) * W + (j + n)] * Kernel[m + 1][n + 1];	// -1 ~ 1 범위 고려하여 +1

				}
			}
			Out[i * W + j] = abs((long)SumProduct) / 3;	// normalize : 0 ~ 765 범위 -> 0 ~ 255
			SumProduct = 0.0;
		}
	}
}

// Sobel 마스크 X : X 방향으로 화소값 변화 지점 검출
void Sobel_X_Conv(BYTE* Img, BYTE* Out, int W, int H)
{
	double Kernel[3][3] = { -1.0, 0.0, 1.0,
							-2.0, 0.0, 2.0,
							-1.0, 0.0, 1.0 };
	double SumProduct = 0.0;					// 컨볼루션 계산 누적합 저장 변수

	for (int i = 1; i < H - 1; i++)				// Y좌표(행), 마진 고려하여 1 감소
	{
		for (int j = 1; j < W - 1; j++)			// X좌표(열)
		{
			for (int m = -1; m <= 1; m++)		// 커널의 행, [i][j] 중심으로 9칸 순회
			{
				for (int n = -1; n <= 1; n++)	// 커널의 열
				{
					SumProduct += Img[(i + m) * W + (j + n)] * Kernel[m + 1][n + 1];	// -1 ~ 1 범위 고려하여 +1

				}
			}
			Out[i * W + j] = abs((long)SumProduct) / 4;	// normalize : 0 ~ 1020 범위 -> 0 ~ 255
			SumProduct = 0.0;
		}
	}
}

// Sobel 마스크 Y : Y 방향으로 화소값 변화 지점 검출
void Sobel_Y_Conv(BYTE* Img, BYTE* Out, int W, int H)
{
	double Kernel[3][3] = { -1.0, -2.0, -1.0,
							0.0, 0.0, 0.0,
							1.0, 2.0, 1.0 };
	double SumProduct = 0.0;					// 컨볼루션 계산 누적합 저장 변수

	for (int i = 1; i < H - 1; i++)				// Y좌표(행), 마진 고려하여 1 감소
	{
		for (int j = 1; j < W - 1; j++)			// X좌표(열)
		{
			for (int m = -1; m <= 1; m++)		// 커널의 행, [i][j] 중심으로 9칸 순회
			{
				for (int n = -1; n <= 1; n++)	// 커널의 열
				{
					SumProduct += Img[(i + m) * W + (j + n)] * Kernel[m + 1][n + 1];	// -1 ~ 1 범위 고려하여 +1

				}
			}
			Out[i * W + j] = abs((long)SumProduct) / 4;	// normalize : 0 ~ 1020 범위 -> 0 ~ 255
			SumProduct = 0.0;
		}
	}
}

// 라플라시안 마스크 : 영상 경계 검출 마스크 (x, y 경계 모두 검출하나 노이즈에 민감)
void Laplace_Conv(BYTE* Img, BYTE* Out, int W, int H)
{
	double Kernel[3][3] = { -1.0, -1.0, -1.0,
							-1.0, 8.0, -1.0,
							-1.0, -1.0, -1.0 };
	double SumProduct = 0.0;					// 컨볼루션 계산 누적합 저장 변수

	for (int i = 1; i < H - 1; i++)				// Y좌표(행), 마진 고려하여 1 감소
	{
		for (int j = 1; j < W - 1; j++)			// X좌표(열)
		{
			for (int m = -1; m <= 1; m++)		// 커널의 행, [i][j] 중심으로 9칸 순회
			{
				for (int n = -1; n <= 1; n++)	// 커널의 열
				{
					SumProduct += Img[(i + m) * W + (j + n)] * Kernel[m + 1][n + 1];	// -1 ~ 1 범위 고려하여 +1

				}
			}
			Out[i * W + j] = abs((long)SumProduct) / 8;	// normalize : 0 ~ 2040 범위 -> 0 ~ 255
			SumProduct = 0.0;
		}
	}
}

// 라플라시안 마스크로 경계 검출하면서 원래 영상 밝기 유지(경계, 노이즈 등 고주파 성분 강조)
void Laplace_Conv_DC(BYTE* Img, BYTE* Out, int W, int H)
{
	double Kernel[3][3] = { -1.0, -1.0, -1.0,
							-1.0, 9.0, -1.0,	// 라플라시안에서 센터 값만 9로 변경 -> 커널 합 = 1 -> 영상 원래 밝기 유지되며 컨볼루션
							-1.0, -1.0, -1.0 };
	double SumProduct = 0.0;					// 컨볼루션 계산 누적합 저장 변수

	for (int i = 1; i < H - 1; i++)				// Y좌표(행), 마진 고려하여 1 감소
	{
		for (int j = 1; j < W - 1; j++)			// X좌표(열)
		{
			for (int m = -1; m <= 1; m++)		// 커널의 행, [i][j] 중심으로 9칸 순회
			{
				for (int n = -1; n <= 1; n++)	// 커널의 열
				{
					SumProduct += Img[(i + m) * W + (j + n)] * Kernel[m + 1][n + 1];	// -1 ~ 1 범위 고려하여 +1

				}
			}
			if(SumProduct > 255.0)	Out[i*W+j] = 255;
			else if (SumProduct <0.0)	Out[i * W + j] = 0;
			else Out[i * W + j] = (BYTE)SumProduct;
			SumProduct = 0.0;
		}
	}
}

// 5주차
// 스왑 함수
void swap(BYTE* a, BYTE* b)
{
	BYTE temp = *a;
	*a = *b;
	*b = temp;
}

// 중간값 필터링
BYTE Median(BYTE* arr, int size)
{	
	const int S = size;
	//bubble sorting
	for (int i = 0; i < size - 1; i++)		 // pivot index
	{
		for (int j = i + 1; j < size; j++)	 // 비교대상 index
		{
			if (arr[i] > arr[j])	swap(&arr[i], &arr[j]);
		}
	}
	return arr[size/2];
}

// 최대값 필터링
BYTE MaxPooling(BYTE* arr, int size)
{	
	const int S = size;
	//bubble sorting
	for (int i = 0; i < size - 1; i++)		 // pivot index
	{
		for (int j = i + 1; j < size; j++)	 // 비교대상 index
		{
			if (arr[i] > arr[j])	swap(&arr[i], &arr[j]);
		}
	}
	return arr[S-1];
}

// 최소값 필터링
BYTE MinPooling(BYTE* arr, int size)
{	
	const int S = size;
	//bubble sorting
	for (int i = 0; i < size - 1; i++)		 // pivot index
	{
		for (int j = i + 1; j < size; j++)	 // 비교대상 index
		{
			if (arr[i] > arr[j])	swap(&arr[i], &arr[j]);
		}
	}
	return arr[0];
}

int main()
{
	BITMAPFILEHEADER hf;		// 14바이트
	BITMAPINFOHEADER hInfo;		// 40바이트
	RGBQUAD hRGB[256];			// 1024바이트
	FILE* fp;
	fp = fopen("lenna_impulse.bmp", "rb");
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

	/* 영상 처리 부분 시작 */

	// 3주차
	// 영상 반전 
	//InverseImage(Image, Output, hInfo.biWidth, hInfo.biHeight);

	// 영상 밝기 조절
	//BrightnessAdj(Image, Output, hInfo.biWidth, hInfo.biHeight, 70);

	// 영상 대비 조절
	//ContrastAdj(Image, Output, hInfo.biWidth, hInfo.biHeight, 0.5);

	// 히스토그램 구하기
	//int Histo[256] = { 0 };	// 영상의 히스토그램 
	//ObtainHistogram(Image, Histo, hInfo.biWidth, hInfo.biHeight);
	
	// 누적 히스토그램 구하기
	//int AHisto[256] = { 0 }; // 영상의 누적히스토그램
	//ObtainAHistogram(Histo, AHisto);

	// 히스토그램 스트레칭
	//HistogramStretching(Image, Output, Histo, hInfo.biWidth, hInfo.biHeight);
	
	// 히스토그램 평활화
	//HistogramEqualization(Image, Output, AHisto, hInfo.biWidth, hInfo.biHeight);

	// 영상 이진화
	//int Thres = GozalezBinThresh(Histo); // 임계값 지정(Gonzalez, Woods 방법)
	//Binarization(Image, Output, hInfo.biWidth, hInfo.biHeight, Thres);
	
	// 4주차
	//애버리지 컨볼루션 (박스 평활화)
	//AverageConv(Image, Output, hInfo.biWidth, hInfo.biHeight);
	
	// 가우시안 컨볼루션 (가우시안 평활화)
	//GaussAvrConv(Image, Output, hInfo.biWidth, hInfo.biHeight);

	// Prewitt 마스크 X 하여 경계 검출 후 이진화 하여 경계 판단
	//Prewitt_X_Conv(Image, Output, hInfo.biWidth, hInfo.biHeight);
	//Binarization(Output, Output, hInfo.biWidth, hInfo.biHeight, 50);
	 
	// Prewitt 마스크 Y 하여 경계 검출 후 이진화 하여 경계 판단
	//Prewitt_Y_Conv(Image, Output, hInfo.biWidth, hInfo.biHeight);
	//Binarization(Output, Output, hInfo.biWidth, hInfo.biHeight, 50);

	// Prewitt X 후 Y
	//Prewitt_X_Conv(Image, Temp, hInfo.biWidth, hInfo.biHeight);
	//Prewitt_Y_Conv(Image, Output, hInfo.biWidth, hInfo.biHeight);
	//for (int i = 0; i < ImgSize; i++)	if (Temp[i] > Output[i])	Output[i] = Temp[i];
	//Binarization(Output, Output, hInfo.biWidth, hInfo.biHeight, 40);

	// Sobel X 후 Y
	//Sobel_X_Conv(Image, Temp, hInfo.biWidth, hInfo.biHeight);
	//Sobel_Y_Conv(Image, Output, hInfo.biWidth, hInfo.biHeight);
	//for (int i = 0; i < ImgSize; i++)	if (Temp[i] > Output[i])	Output[i] = Temp[i];
	//Binarization(Output, Output, hInfo.biWidth, hInfo.biHeight, 40);

	// 라플라시안 마스크
	//Laplace_Conv(Image, Output, hInfo.biWidth, hInfo.biHeight);

	// 라플라시안 마스크로 경계 검출하면서 원래 영상 밝기 유지(경계, 노이즈 등 고주파 성분 강조)
	//Laplace_Conv_DC(Image, Output, hInfo.biWidth, hInfo.biHeight);

	// 가우시안 평활화 후 라플라시안 마스크로 경계 검출하면서 원래 영상 밝기 유지
	//GaussAvrConv(Image, Temp, hInfo.biWidth, hInfo.biHeight);
	//Laplace_Conv_DC(Temp, Output, hInfo.biWidth, hInfo.biHeight);

	// 5주차
	// 중간값 필터링 (lenna_impulse 임펄스 노이즈 영상 개선)
	//BYTE temp[9];
	//int W = hInfo.biWidth, H = hInfo.biHeight;
	//int i, j;
	//for (i = 1; i < H - 1; i++) {
	//	for (j = 1; j < W - 1; j++) {
	//		temp[0] = Image[(i - 1) * W + j-1];			// 1
	//		temp[1] = Image[(i - 1) * W + j];			// 2
	//		temp[2] = Image[(i - 1) * W + j+1];			// 3
	//		temp[3] = Image[i * W + j-1];				// 4
	//		temp[4] = Image[i * W + j];					// 센터
	//		temp[5] = Image[i * W + j+1];				// 6
	//		temp[6] = Image[(i + 1) * W + j-1];			// 7
	//		temp[7] = Image[(i + 1) * W + j];			// 8
	//		temp[8] = Image[(i + 1) * W + j+1];			// 9
	//		//Output[i * W + j] = Median(temp, 9);		// 중간값 필터링
	//		//Output[i * W + j] = MaxPooling(temp, 9);	// 최대값 필터링 (영상에 페퍼 노이즈 존재 시 사용)
	//		Output[i * W + j] = MinPooling(temp, 9);	// 최소값 필터링 (영상에 솔트 노이즈 존재 시 사용)
	//	}
	//}

	// 5주차 과제 HW3 - 윈도우 사이즈에 따른 중간값 필터링 결과 
	/* Median filtering */

	int Length = 9;										// 마스크 한 변의 길이
	int Margin = Length / 2;							// 영상의 마진
	int WSize = Length * Length;						// 마스크(윈도우) 크기
	BYTE* temp = (BYTE*)malloc(sizeof(BYTE) * WSize);	// 영상에서 마스크가 덮은 영역 담아 둘 임시 배열
	int W = hInfo.biWidth, H = hInfo.biHeight;			// 원본 영상 가로, 세로 크기
	int i, j, m, n;										// for문 제어용 변수
	for (i = Margin; i < H - Margin; i++) {				// 마진 고려하여 영상 순회
		for (j = Margin; j < W - Margin; j++) {			// 마진 고려하여 영상 순회
			for (m = -Margin; m <= Margin; m++) {		// 현 위치를 중심으로 윈도우 순회
				for (n = -Margin; n <= Margin; n++) {	// 현 위치를 중심으로 윈도우 순회
					temp[(m + Margin) * Length + (n + Margin)] = Image[(i + m) * W + j + n];	// 윈도우 내 값들의 중간값 필터링 위해 임시 배열에 저장 
				}
			}
			Output[i * W + j] = Median(temp, WSize);	// 주어진 마진, 마스크 크기에 따른 중간값 필터링 결과를 Output에 저장
		}
	}
	free(temp);

	/* Median filtering */

	// 평균값 필터링 (lenna_gauss 가우시안 노이즈 영상 개선)
	//AverageConv(Image, Output, hInfo.biWidth, hInfo.biHeight);

	/* 영상 처리 부분 끝 */

	/* 출력 */
	SaveBMPFile(hf, hInfo, hRGB, Output, hInfo.biWidth, hInfo.biHeight, "median_9.bmp");

	free(Image);
	free(Temp);
	free(Output);
	return 0;
}