#pragma warning(disable:4996)
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <math.h>

// 2차원 배열 동적할당 위함
unsigned char** imageMatrix;

// 이진영상에서 
unsigned char blankPixel = 255, imagePixel = 0;

typedef struct {
	int row, col;
}pixel;
//////////////

void InverseImage(BYTE* Img, BYTE *Out, int W, int H)
{
	int ImgSize = W * H;
	for (int i = 0; i < ImgSize; i++)
	{
		Out[i] = 255 - Img[i];
	}
}
void BrightnessAdj(BYTE* Img, BYTE* Out, int W, int H, int Val)
{
	int ImgSize = W * H;
	for (int i = 0; i < ImgSize; i++)
	{
		if (Img[i] + Val > 255)
		{
			Out[i] = 255;
		}
		else if (Img[i] + Val < 0)
		{
			Out[i] = 0;
		}
		else 	Out[i] =Img[i] + Val;
	}
}
void ContrastAdj(BYTE* Img, BYTE* Out, int W, int H, double Val)
{
	int ImgSize = W * H;
	for (int i = 0; i < ImgSize; i++)
	{
		if (Img[i] * Val > 255.0)
		{
			Out[i] = 255;
		}
		else 	Out[i] = (BYTE)(Img[i] * Val);
	}
}

void ObtainHistogram(BYTE* Img, int* Histo, int W, int H)
{
	int ImgSize = W * H;
	for (int i = 0; i < ImgSize; i++) {
		Histo[Img[i]]++;
	}
}

void ObtainAHistogram(int* Histo, int* AHisto)
{
	for (int i = 0; i < 256; i++) {
		for (int j = 0; j <= i; j++) {
			AHisto[i] += Histo[j];
		}
	}
}

void HistogramStretching(BYTE * Img, BYTE * Out, int * Histo, int W, int H)
{
	int ImgSize = W * H;
	BYTE Low, High;
	for (int i = 0; i < 256; i++) {
		if (Histo[i] != 0) {
			Low = i;
			break;
		}
	}
	for (int i = 255; i >= 0; i--) {
		if (Histo[i] != 0) {
			High = i;
			break;
		}
	}
	for (int i = 0; i < ImgSize; i++) {
		Out[i] = (BYTE)((Img[i] - Low) / (double)(High - Low) * 255.0);
	}
}
void HistogramEqualization(BYTE* Img, BYTE* Out, int* AHisto, int W, int H)
{
	int ImgSize = W * H;
	int Nt = W * H, Gmax = 255;
	double Ratio = Gmax / (double)Nt;
	BYTE NormSum[256];
	for (int i = 0; i < 256; i++) {
		NormSum[i] = (BYTE)(Ratio * AHisto[i]);
	}
	for (int i = 0; i < ImgSize; i++)
	{
		Out[i] = NormSum[Img[i]];
	}
}

void Binarization(BYTE * Img, BYTE * Out, int W, int H, BYTE Threshold)
{
	int ImgSize = W * H;
	for (int i = 0; i < ImgSize; i++) {
		if (Img[i] < Threshold) Out[i] = 0;
		else Out[i] = 255;
	}
}

//int GozalezBinThresh()
//{
//
//}

void AverageConv(BYTE* Img, BYTE* Out, int W, int H) // 박스평활화
{
	double Kernel[3][3] = {0.11111, 0.11111, 0.11111,
										0.11111, 0.11111, 0.11111,
										0.11111, 0.11111, 0.11111	};
	double SumProduct = 0.0;
	for (int i = 1; i < H-1; i++) { // Y좌표 (행)
		for (int j = 1; j < W-1; j++) { // X좌표 (열)
			for (int m = -1; m <= 1; m++) { // Kernel 행
				for (int n = -1; n <= 1; n++) { // Kernel 열
					SumProduct += Img[(i+m)*W + (j+n)] * Kernel[m+1][n+1];
				}
			}
			Out[i * W + j] = (BYTE)SumProduct;
			SumProduct = 0.0;
		}
	}
}

void GaussAvrConv(BYTE* Img, BYTE* Out, int W, int H) // 가우시안평활화
{
	double Kernel[3][3] = {0.0625, 0.125, 0.0625,
										0.125, 0.25, 0.125,
										0.0625, 0.125, 0.0625 };
	double SumProduct = 0.0;
	for (int i = 1; i < H - 1; i++) { // Y좌표 (행)
		for (int j = 1; j < W - 1; j++) { // X좌표 (열)
			for (int m = -1; m <= 1; m++) { // Kernel 행
				for (int n = -1; n <= 1; n++) { // Kernel 열
					SumProduct += Img[(i + m) * W + (j + n)] * Kernel[m + 1][n + 1];
				}
			}
			Out[i * W + j] = (BYTE)SumProduct;
			SumProduct = 0.0;
		}
	}
}

void Prewitt_X_Conv(BYTE* Img, BYTE* Out, int W, int H) // Prewitt 마스크 X
{
	double Kernel[3][3] = { -1.0, 0.0, 1.0,
										-1.0, 0.0, 1.0,
										-1.0, 0.0, 1.0 };
	double SumProduct = 0.0;
	for (int i = 1; i < H - 1; i++) { // Y좌표 (행)
		for (int j = 1; j < W - 1; j++) { // X좌표 (열)
			for (int m = -1; m <= 1; m++) { // Kernel 행
				for (int n = -1; n <= 1; n++) { // Kernel 열
					SumProduct += Img[(i + m) * W + (j + n)] * Kernel[m + 1][n + 1];
				}
			}
			// 0 ~ 765  =====> 0 ~ 255
			Out[i * W + j] = abs((long)SumProduct) / 3;
			SumProduct = 0.0;
		}
	}
}

void Prewitt_Y_Conv(BYTE* Img, BYTE* Out, int W, int H) // Prewitt 마스크 X
{
	double Kernel[3][3] = { -1.0, -1.0, -1.0,
										0.0, 0.0, 0.0,
										1.0, 1.0, 1.0 };
	double SumProduct = 0.0;
	for (int i = 1; i < H - 1; i++) { // Y좌표 (행)
		for (int j = 1; j < W - 1; j++) { // X좌표 (열)
			for (int m = -1; m <= 1; m++) { // Kernel 행
				for (int n = -1; n <= 1; n++) { // Kernel 열
					SumProduct += Img[(i + m) * W + (j + n)] * Kernel[m + 1][n + 1];
				}
			}
			// 0 ~ 765  =====> 0 ~ 255
			Out[i * W + j] = abs((long)SumProduct) / 3;
			SumProduct = 0.0;
		}
	}
}

void Sobel_X_Conv(BYTE* Img, BYTE* Out, int W, int H) // Prewitt 마스크 X
{
	double Kernel[3][3] = { -1.0, 0.0, 1.0,
										-2.0, 0.0, 2.0,
										-1.0, 0.0, 1.0 };
	double SumProduct = 0.0;
	for (int i = 1; i < H - 1; i++) { // Y좌표 (행)
		for (int j = 1; j < W - 1; j++) { // X좌표 (열)
			for (int m = -1; m <= 1; m++) { // Kernel 행
				for (int n = -1; n <= 1; n++) { // Kernel 열
					SumProduct += Img[(i + m) * W + (j + n)] * Kernel[m + 1][n + 1];
				}
			}
			// 0 ~ 1020  =====> 0 ~ 255
			Out[i * W + j] = abs((long)SumProduct) / 4;
			SumProduct = 0.0;
		}
	}
}

void Sobel_Y_Conv(BYTE* Img, BYTE* Out, int W, int H) // Prewitt 마스크 X
{
	double Kernel[3][3] = { -1.0, -2.0, -1.0,
										0.0, 0.0, 0.0,
										1.0, 2.0, 1.0 };
	double SumProduct = 0.0;
	for (int i = 1; i < H - 1; i++) { // Y좌표 (행)
		for (int j = 1; j < W - 1; j++) { // X좌표 (열)
			for (int m = -1; m <= 1; m++) { // Kernel 행
				for (int n = -1; n <= 1; n++) { // Kernel 열
					SumProduct += Img[(i + m) * W + (j + n)] * Kernel[m + 1][n + 1];
				}
			}
			// 0 ~ 765  =====> 0 ~ 255
			Out[i * W + j] = abs((long)SumProduct) / 4;
			SumProduct = 0.0;
		}
	}
}

void Laplace_Conv(BYTE* Img, BYTE* Out, int W, int H) // Prewitt 마스크 X
{
	double Kernel[3][3] = { -1.0, -1.0, -1.0,
										-1.0, 8.0, -1.0,
										-1.0, -1.0, -1.0 };
	double SumProduct = 0.0;
	for (int i = 1; i < H - 1; i++) { // Y좌표 (행)
		for (int j = 1; j < W - 1; j++) { // X좌표 (열)
			for (int m = -1; m <= 1; m++) { // Kernel 행
				for (int n = -1; n <= 1; n++) { // Kernel 열
					SumProduct += Img[(i + m) * W + (j + n)] * Kernel[m + 1][n + 1];
				}
			}
			// 0 ~ 2040  =====> 0 ~ 255
			Out[i * W + j] = abs((long)SumProduct) / 8;
			SumProduct = 0.0;
		}
	}
}

void Laplace_Conv_DC(BYTE* Img, BYTE* Out, int W, int H) // Prewitt 마스크 X
{
	double Kernel[3][3] = { -1.0, -1.0, -1.0,
										-1.0, 9.0, -1.0,
										-1.0, -1.0, -1.0 };
	double SumProduct = 0.0;
	for (int i = 1; i < H - 1; i++) { // Y좌표 (행)
		for (int j = 1; j < W - 1; j++) { // X좌표 (열)
			for (int m = -1; m <= 1; m++) { // Kernel 행
				for (int n = -1; n <= 1; n++) { // Kernel 열
					SumProduct += Img[(i + m) * W + (j + n)] * Kernel[m + 1][n + 1];
				}
			}
			//Out[i * W + j] = abs((long)SumProduct) / 8;
			if (SumProduct > 255.0) Out[i * W + j] = 255;
			else if (SumProduct < 0.0) Out[i * W + j] = 0;
			else Out[i * W + j] = (BYTE)SumProduct;
			SumProduct = 0.0;
		}
	}
}

void SaveBMPFile(BITMAPFILEHEADER hf, BITMAPINFOHEADER hInfo, 
	RGBQUAD* hRGB, BYTE* Output, int W, int H, const char* FileName)
{
	FILE * fp = fopen(FileName, "wb");
	if (hInfo.biBitCount == 24) {
		fwrite(&hf, sizeof(BYTE), sizeof(BITMAPFILEHEADER), fp);
		fwrite(&hInfo, sizeof(BYTE), sizeof(BITMAPINFOHEADER), fp);
		fwrite(Output, sizeof(BYTE), W * H * 3, fp);
	}
	else if (hInfo.biBitCount == 8) {
		fwrite(&hf, sizeof(BYTE), sizeof(BITMAPFILEHEADER), fp);
		fwrite(&hInfo, sizeof(BYTE), sizeof(BITMAPINFOHEADER), fp);
		fwrite(hRGB, sizeof(RGBQUAD), 256, fp);
		fwrite(Output, sizeof(BYTE), W * H, fp);
	}
	fclose(fp);
}

void swap(BYTE* a, BYTE* b) 
{
	BYTE temp = *a;
	*a = *b;
	*b = temp;
}

BYTE Median(BYTE* arr, int size)
{
	// 오름차순 정렬
	const int S = size;
	for (int i = 0; i < size - 1; i++) // pivot index
	{
		for (int j = i + 1; j < size; j++) // 비교대상 index
		{
			if (arr[i] > arr[j]) 	swap(&arr[i], &arr[j]);
		}
	}
	return arr[S/2];
}

BYTE MaxPooling(BYTE* arr, int size)
{
	// 오름차순 정렬
	const int S = size;
	for (int i = 0; i < size - 1; i++) // pivot index
	{
		for (int j = i + 1; j < size; j++) // 비교대상 index
		{
			if (arr[i] > arr[j]) 	swap(&arr[i], &arr[j]);
		}
	}
	return arr[S-1];
}

BYTE MinPooling(BYTE* arr, int size)
{
	// 오름차순 정렬
	const int S = size;
	for (int i = 0; i < size - 1; i++) // pivot index
	{
		for (int j = i + 1; j < size; j++) // 비교대상 index
		{
			if (arr[i] > arr[j]) 	swap(&arr[i], &arr[j]);
		}
	}
	return arr[0];
}

int push(short* stackx, short* stacky, int arr_size, short vx, short vy, int* top)
{
	if (*top >= arr_size) return(-1);
	(*top)++;
	stackx[*top] = vx;
	stacky[*top] = vy;
	return(1);
}

int pop(short* stackx, short* stacky, short* vx, short* vy, int* top)
{
	if (*top == 0) return(-1);
	*vx = stackx[*top];
	*vy = stacky[*top];
	(*top)--;
	return(1);
}


// GlassFire 알고리즘을 이용한 라벨링 함수
void m_BlobColoring(BYTE* CutImage, int height, int width)
{
	int i, j, m, n, top, area, Out_Area, index, BlobArea[1000];
	long k;
	short curColor = 0, r, c;
	//	BYTE** CutImage2;
	Out_Area = 1;
	
	// 스택으로 사용할 메모리 할당
	short* stackx = new short[height * width];
	short* stacky = new short[height * width];
	short* coloring = new short[height * width];

	int arr_size = height * width;

	// 라벨링된 픽셀을 저장하기 위해 메모리 할당

	for (k = 0; k < height * width; k++) coloring[k] = 0;  // 메모리 초기화

	for (i = 0; i < height; i++)
	{
		index = i * width;
		for (j = 0; j < width; j++)
		{
			// 이미 방문한 점이거나 픽셀값이 255가 아니라면 처리 안함
			if (coloring[index + j] != 0 || CutImage[index + j] != 255) continue;
			r = i; c = j; top = 0; area = 1;
			curColor++;

			while (1)
			{
			GRASSFIRE:
				for (m = r - 1; m <= r + 1; m++)
				{
					index = m * width;
					for (n = c - 1; n <= c + 1; n++)
					{
						//관심 픽셀이 영상경계를 벗어나면 처리 안함
						if (m < 0 || m >= height || n < 0 || n >= width) continue;

						if ((int)CutImage[index + n] == 255 && coloring[index + n] == 0)
						{
							coloring[index + n] = curColor; // 현재 라벨로 마크
							if (push(stackx, stacky, arr_size, (short)m, (short)n, &top) == -1) continue;
							r = m; c = n; area++;
							goto GRASSFIRE;
						}
					}
				}
				if (pop(stackx, stacky, &r, &c, &top) == -1) break;
			}
			if (curColor < 1000) BlobArea[curColor] = area;
		}
	}

	float grayGap = 255.0f / (float)curColor;

	// 가장 면적이 넓은 영역을 찾아내기 위함 
	for (i = 1; i <= curColor; i++)
	{
		if (BlobArea[i] >= BlobArea[Out_Area]) Out_Area = i;
	}
	// CutImage 배열 클리어~
	for (k = 0; k < width * height; k++) CutImage[k] = 255;

	// coloring에 저장된 라벨링 결과중 (Out_Area에 저장된) 영역이 가장 큰 것만 CutImage에 저장
	for (k = 0; k < width * height; k++)
	{
		if (coloring[k] == Out_Area) CutImage[k] = 0;  // 가장 큰 것만 저장 (size filtering)
		//if (BlobArea[coloring[k]] > 500) CutImage[k] = 0;  // 특정 면적이상되는 영역만 출력
		//CutImage[k] = (unsigned char)(coloring[k] * grayGap);
	}

	delete[] coloring;
	delete[] stackx;
	delete[] stacky;
}
// 라벨링 후 가장 넓은 영역에 대해서만 뽑아내는 코드 포함

void BinaryImageEdgeDetection(BYTE* Bin, BYTE* Out, int W, int H)
{
	for (int i = 0; i < H; i++) {
		for (int j = 0; j < W; j++) {
			if (Bin[i * W + j] == 0) // 전경화소라면
			{
				if (!(Bin[(i - 1) * W + j] == 0 && Bin[(i + 1) * W + j] == 0 &&
					Bin[i * W + j - 1] == 0 && Bin[i * W + j + 1] == 0)) // 4방향 화소 중 하나라도 전경이 아니라면
					Out[i * W + j] = 255;
			}
		}
	}
}

BYTE DetermThGonzalez(int* H)
{
	BYTE ep = 3;
	BYTE Low, High;
	BYTE Th, NewTh;
	int G1 = 0, G2 = 0, cnt1 = 0, cnt2 = 0, mu1, mu2;

	for (int i = 0; i < 256; i++) {
		if (H[i] != 0) {
			Low = i;
			break;
		}
	}
	for (int i = 255; i >= 0; i--) {
		if (H[i] != 0) {
			High = i;
			break;
		}
	}

	Th = (Low + High) / 2;

	//printf("%d\n", Th);
	while(1)
	{
		for (int i = Th + 1; i <= High; i++) {
			G1 += (H[i] * i);
			cnt1 += H[i];
		}
		for (int i = Low; i <= Th; i++) {
			G2 += (H[i] * i);
			cnt2 += H[i];
		}
		if (cnt1 == 0) cnt1 = 1;
		if (cnt2 == 0) cnt2 = 1;

		mu1 = G1 / cnt1;
		mu2 = G2 / cnt2;

		NewTh = (mu1 + mu2) / 2;

		if (abs(NewTh - Th) < ep)
		{
			Th = NewTh;
			break;
		}
		else
		{
			Th = NewTh;
		}
		G1 = G2 = cnt1 = cnt2 = 0;
		//printf("%d\n", Th);
	}
	return Th;
}

void VerticalFlip(BYTE * Img, int W, int H)
{
	for (int i = 0; i < H / 2; i++) { // y좌표
		for (int j = 0; j < W; j++) { // x좌표
			swap(&Img[i*W + j], &Img[(H-1-i)*W + j]);
		}
	}
}

void HorizontalFlip(BYTE* Img, int W, int H)
{
	for (int i = 0; i < W / 2; i++) { // x좌표
		for (int j = 0; j < H; j++) { // y좌표
			swap(&Img[j * W + i], &Img[j * W + (W-1-i)]);
		}
	}
}

void Translation(BYTE* Image, BYTE* Output, int W, int H, int Tx, int Ty)
{
	// Translation
	Ty *= -1;
	for (int i = 0; i < H; i++) {
		for (int j = 0; j < W; j++) {
			if ((i + Ty < H && i + Ty >= 0) && (j + Tx < W && j + Tx >= 0))
				Output[(i + Ty) * W + (j + Tx)] = Image[i * W + j];
		}
	}
}

void Scaling(BYTE* Image, BYTE* Output, int W, int H, double SF_X, double SF_Y)
{
	// Scaling
	int tmpX, tmpY;
	for (int i = 0; i < H; i++) {
		for (int j = 0; j < W; j++) {
			tmpX = (int)(j / SF_X);
			tmpY = (int)(i / SF_Y);
			if (tmpY < H && tmpX < W)
				Output[i * W + j] = Image[tmpY * W + tmpX];
		}
	}

}

void Rotation(BYTE* Image, BYTE* Output, int W, int H, int Angle)
{
	int tmpX, tmpY;
	double Radian = Angle * 3.141592 / 180.0;
	for (int i = 0; i < H; i++) {
		for (int j = 0; j < W; j++) {
			tmpX = (int)(cos(Radian) * j + sin(Radian) * i);
			tmpY = (int)(-sin(Radian) * j + cos(Radian) * i);
			if ((tmpY < H && tmpY >= 0) && (tmpX < W && tmpX >= 0))
				Output[i * W + j] = Image[tmpY * W + tmpX];
		}
	}
}

void MedianFiltering(BYTE* Image, BYTE* Output, int W, int H, int size)
{
	int Length = size;  // 마스크의 한 변의 길이
	int Margin = Length / 2;
	int WSize = Length * Length;
	BYTE* temp = (BYTE*)malloc(sizeof(BYTE) * WSize);
	int i, j, m, n;
	for (i = Margin; i < H - Margin; i++) {
		for (j = Margin; j < W - Margin; j++) {
			for (m = -Margin; m <= Margin; m++) {
				for (n = -Margin; n <= Margin; n++) {
					temp[(m + Margin) * Length + (n + Margin)] = Image[(i + m) * W + j + n];
				}
			}
			Output[i * W + j] = Median(temp, WSize);
		}
	}
	free(temp);
}

// Img: 사각형을 그릴 이미지배열, W: 영상 가로사이즈, H: 영상 세로사이즈,
// LU_X: 사각형의 좌측상단 X좌표, LU_Y: 사각형의 좌측상단 Y좌표,
// RD_X: 사각형의 우측하단 X좌표, LU_Y: 사각형의 우측하단 Y좌표.
void DrawRectOutline(BYTE* Img, int W, int H, int LU_X, int LU_Y, int RD_X, int RD_Y)
{
	for (int i = LU_X; i < RD_X; i++) 	Img[LU_Y * W + i] = 255;
	for (int i = LU_X; i < RD_X; i++) 	Img[RD_Y * W + i] = 255;
	for (int i = LU_Y; i < RD_Y; i++) 	Img[i * W + LU_X] = 255;
	for (int i = LU_Y; i < RD_Y; i++) 	Img[i * W + RD_X] = 255;	
}

// Img: 가로/세로 라인을 그릴 이미지배열, W: 영상 가로사이즈, H: 영상 세로사이즈,
// Cx: 가로/세로 라인이 교차되는 지점의 X좌표
// Cy: 가로/세로 라인이 교차되는 지점의 Y좌표
void DrawCrossLine(BYTE* Img, int W, int H, int Cx, int Cy)
{
	for (int i = 0; i < W - 1; i++) // horizontal line
		Img[Cy * W + i] = 255;
	for (int i = 0; i < H - 1; i++) // vertical line
		Img[i * W + Cx] = 255;
}

void Obtain2DCenter(BYTE* Image, int W, int H, int* Cx, int* Cy)
{
	int SumX = 0, SumY = 0;
	int cnt = 0;
	for (int i = 0; i < H; i++) {
		for (int j = 0; j < W; j++) {
			if (Image[i * W + j] == 0) // 동공영역이면
			{
				SumX += j;
				SumY += i;
				cnt++;
			}
		}
	}
	if (cnt == 0) 	cnt = 1;
	*Cx = SumX / cnt;
	*Cy = SumY / cnt;
	//printf("%d    %d\n", Cx, Cy);
}

void Obtain2DBoundingBox(BYTE* Image, int W, int H, int* LUX, int* LUY, int* RDX, int* RDY) 
{
	int flag = 0;
	for (int i = 0; i < H; i++) {
		for (int j = 0; j < W; j++) {
			if (Image[i * W + j] == 0) {
				*LUY = i;
				flag = 1;
				break;
			}
		}
		if (flag == 1) break;
	}
	flag = 0;
	for (int i = H - 1; i >= 0; i--) {
		for (int j = 0; j < W; j++) {
			if (Image[i * W + j] == 0) {
				*RDY = i;
				flag = 1;
				break;
			}
		}
		if (flag == 1) break;
	}
	flag = 0;

	for (int j = 0; j < W; j++) {
		for (int i = 0; i < H; i++) {
			if (Image[i * W + j] == 0) {
				*LUX = j;
				flag = 1;
				break;
			}
		}
		if (flag == 1) break;
	}
	flag = 0;
	for (int j = W - 1; j >= 0; j--) {
		for (int i = 0; i < H; i++) {
			if (Image[i * W + j] == 0) {
				*RDX = j;
				flag = 1;
				break;
			}
		}
		if (flag == 1) break;
	}
}

// 10주차
void FillColor(BYTE* Image, int X, int Y, int W, int H, BYTE R, BYTE G, BYTE B)		// BGR 순서로 들어가 있다
{
	Image[Y * W * 3 + X * 3] = B;		// Blue 성분
	Image[Y * W * 3 + X * 3 + 1] = G;	// Green 성분
	Image[Y * W * 3 + X * 3 + 2] = R;	// Red 성분
}

// 11주차
void RGB2YCbCr(BYTE* Image, BYTE* Y, BYTE* Cb, BYTE* Cr, int W, int H)
{
	for (int i = 0; i < H; i++) { // Y좌표
		for (int j = 0; j < W; j++) { // X좌표
			Y[i * W + j] = (BYTE)(0.299 * Image[i * W * 3 + j * 3 + 2] + 0.587 * Image[i * W * 3 + j * 3 + 1] + 0.114 * Image[i * W * 3 + j * 3]);
			Cb[i * W + j] = (BYTE)(-0.16874 * Image[i * W * 3 + j * 3 + 2] -0.3313 * Image[i * W * 3 + j * 3 + 1] + 0.5 * Image[i * W * 3 + j * 3] + 128.0);
			Cr[i * W + j] = (BYTE)(0.5 * Image[i * W * 3 + j * 3 + 2] - 0.4187 * Image[i * W * 3 + j * 3 + 1] - 0.0813 * Image[i * W * 3 + j * 3] + 128.0);
		}
	}
}

// 12주차
// 침식
void Erosion(BYTE* Image, BYTE* Output, int W, int H)
{
	for (int i = 1; i < H - 1; i++) {	// 이미지 전체 탐색 (상하좌우 탐색하므로 마진 위해 1부터 시작)
		for (int j = 1; j < W - 1; j++) {
			if (Image[i * W + j] == 255) // 전경화소라면
			{
				if (!(Image[(i - 1) * W + j] == 255 &&
					Image[(i + 1) * W + j] == 255 &&
					Image[i * W + j - 1] == 255 &&
					Image[i * W + j + 1] == 255)) // 4주변화소가 모두 전경화소가 아니라면
					Output[i * W + j] = 0; 
				else Output[i * W + j] = 255;
			}
			else Output[i * W + j] = 0;
		}
	}
}

// 팽창
void Dilation(BYTE* Image, BYTE* Output, int W, int H)
{
	for (int i = 1; i < H - 1; i++) {
		for (int j = 1; j < W - 1; j++) {
			if (Image[i * W + j] == 0) // 배경화소라면
			{
				if (!(Image[(i - 1) * W + j] == 0 &&
					Image[(i + 1) * W + j] == 0 &&
					Image[i * W + j - 1] == 0 &&
					Image[i * W + j + 1] == 0)) // 4주변화소가 모두 배경화소가 아니라면
					Output[i * W + j] = 255;
				else Output[i * W + j] = 0;
			}
			else Output[i * W + j] = 255;
		}
	}
}

// ZhangSuen 세선화(Thinning) 알고리즘 -> 지문인식 등에 사용 (지문의 경로, 분기점 파악)
int getBlackNeighbours(int row, int col) { // 8방향 검사해서 전경(imagePixel=0; zhangsuen은 전경을 검정색으로 구현해놓음) 개수만큼 sum 누적하여 리턴

	int i, j, sum = 0;

	for (i = -1; i <= 1; i++) {
		for (j = -1; j <= 1; j++) {
			if (i != 0 || j != 0)
				sum += (imageMatrix[row + i][col + j] == imagePixel);
		}
	}

	return sum;
}

int getBWTransitions(int row, int col) { // 중심 픽셀 기준으로 2번방 픽셀부터 시계방향으로 가면서 255->0 값 변하는 곳 개수 리턴
	return 	((imageMatrix[row - 1][col] == blankPixel && imageMatrix[row - 1][col + 1] == imagePixel)	// 2, 3 비교
		+ (imageMatrix[row - 1][col + 1] == blankPixel && imageMatrix[row][col + 1] == imagePixel)		// 3, 6 비교
		+ (imageMatrix[row][col + 1] == blankPixel && imageMatrix[row + 1][col + 1] == imagePixel)		// 6, 9 비교
		+ (imageMatrix[row + 1][col + 1] == blankPixel && imageMatrix[row + 1][col] == imagePixel)		// 9, 8 비교
		+ (imageMatrix[row + 1][col] == blankPixel && imageMatrix[row + 1][col - 1] == imagePixel)		// 8, 7 비교
		+ (imageMatrix[row + 1][col - 1] == blankPixel && imageMatrix[row][col - 1] == imagePixel)		// 7, 4 비교
		+ (imageMatrix[row][col - 1] == blankPixel && imageMatrix[row - 1][col - 1] == imagePixel)		// 7, 1 비교
		+ (imageMatrix[row - 1][col - 1] == blankPixel && imageMatrix[row - 1][col] == imagePixel));	// 1, 2 비교
}

int zhangSuenTest1(int row, int col) {	// zhangSuen 세선화 알고리즘 좌하함수
	int neighbours = getBlackNeighbours(row, col);	// 이웃 전경화소 개수 구하기

	return ((2 <= neighbours && neighbours <= 6)
		&& (getBWTransitions(row, col) == 1)																									// 주변 픽셀에서 전경->배경 전환이 1회만 이루어지고
		&& (imageMatrix[row - 1][col] == blankPixel || imageMatrix[row][col + 1] == blankPixel || imageMatrix[row + 1][col] == blankPixel)		// 2, 6, 8번 중 배경 화소가 있고
		&& (imageMatrix[row][col + 1] == blankPixel || imageMatrix[row + 1][col] == blankPixel || imageMatrix[row][col - 1] == blankPixel));	// 6, 8, 4번 중에 배경화소가 있으면 1 리턴 
}

int zhangSuenTest2(int row, int col) {	// zhangSuen 세선화 알고리즘 우상함수
	int neighbours = getBlackNeighbours(row, col);	// 이웃 전경화소 개수 구하기

	return ((2 <= neighbours && neighbours <= 6)																								// 8방향 검사하여 전경화소가 2이상 6이하 이면서
		&& (getBWTransitions(row, col) == 1)																									// 주변 픽셀에서 전경->배경 전환이 1회만 이루어지고
		&& (imageMatrix[row - 1][col] == blankPixel || imageMatrix[row][col + 1] == blankPixel || imageMatrix[row][col - 1] == blankPixel)		// 2, 6, 4 중 배경 화소가 있고
		&& (imageMatrix[row - 1][col] == blankPixel || imageMatrix[row + 1][col] == blankPixel || imageMatrix[row][col + 1] == blankPixel));	// 2, 8, 6 중에 배경화소가 있으면 1 리턴
}

void zhangSuen(unsigned char* image, unsigned char* output, int rows, int cols) {

	int startRow = 1, startCol = 1, endRow, endCol, i, j, count, processed;

	pixel* markers;

	imageMatrix = (unsigned char**)malloc(rows * sizeof(unsigned char*));

	for (i = 0; i < rows; i++) {
		imageMatrix[i] = (unsigned char*)malloc((cols + 1) * sizeof(unsigned char));
		for (int k = 0; k < cols; k++) imageMatrix[i][k] = image[i * cols + k];
	}

	endRow = rows - 2;
	endCol = cols - 2;
	do {
		markers = (pixel*)malloc((endRow - startRow + 1) * (endCol - startCol + 1) * sizeof(pixel));
		count = 0;

		for (i = startRow; i <= endRow; i++) {
			for (j = startCol; j <= endCol; j++) {
				if (imageMatrix[i][j] == imagePixel && zhangSuenTest1(i, j) == 1) {
					markers[count].row = i;
					markers[count].col = j;
					count++;
				}
			}
		}

		processed = (count > 0);

		for (i = 0; i < count; i++) {
			imageMatrix[markers[i].row][markers[i].col] = blankPixel;
		}

		free(markers);
		markers = (pixel*)malloc((endRow - startRow + 1) * (endCol - startCol + 1) * sizeof(pixel));
		count = 0;

		for (i = startRow; i <= endRow; i++) {
			for (j = startCol; j <= endCol; j++) {
				if (imageMatrix[i][j] == imagePixel && zhangSuenTest2(i, j) == 1) {
					markers[count].row = i;
					markers[count].col = j;
					count++;
				}
			}
		}

		if (processed == 0)
			processed = (count > 0);

		for (i = 0; i < count; i++) {
			imageMatrix[markers[i].row][markers[i].col] = blankPixel;
		}

		free(markers);
	} while (processed == 1);


	for (i = 0; i < rows; i++) {
		for (j = 0; j < cols; j++) {
			output[i * cols + j] = imageMatrix[i][j];
		}
	}
}

// 12주차 실습과제
void FeatureExtractThinImage(BYTE* Image, BYTE* Output, int W, int H)	// Image: 세선화된 이진 영상, Output : 분기점 및 끝점이 표시된 영상
{
	for (int i = 1; i < H - 1; i++) {				// 이미지 전체 탐색하며 Output에 Image 복사
		for (int j = 1; j < W - 1; j++) {
			Output[i * W + j] = Image[i * W + j];
		}
	}

	for (int i = 1; i < H - 1; i++) {				// 다시 이미지 전체 탐색
		for (int j = 1; j < W - 1; j++) {
			if (Image[i * W + j] == imagePixel) {	// 전경화소이면서
				int neighbours = getBlackNeighbours(i, j);
				if (neighbours >= 3) {				// 주변 화소 중 3개 이상이 전경화소 이면 회색으로 분기점 표시
					Output[i * W + j] = 190;
				}
			}
		}
	}

	for (int i = 1; i < H - 1; i++) {				// 다시 이미지 전체 탐색
		for (int j = 1; j < W - 1; j++) {
			if (Image[i * W + j] == imagePixel) {	// 전경화소이면서 
				int neighbours = getBlackNeighbours(i, j);
				if (neighbours == 1) {			// 주변 화소 중 1개 이하가 전경화소 이면 박스로 끝점 표시
					Output[(i - 1) * W + j] = 100;	//상
					Output[(i + 1) * W + j] = 100;	//하
					Output[i * W + (j - 1)] = 100;	//좌
					Output[i * W + (j + 1)] = 100;	//우
					Output[(i - 1) * W + (j - 1)] = 100;	//좌상
					Output[(i - 1) * W + (j + 1)] = 100;	//우상
					Output[(i + 1) * W + (j - 1)] = 100;	//좌하
					Output[(i + 1) * W + (j + 1)] = 100;	//우하
				}
			}
		}
	}
}

int main()
{
	BITMAPFILEHEADER hf; // 14바이트
	BITMAPINFOHEADER hInfo; // 40바이트
	RGBQUAD hRGB[256]; // 1024바이트
	FILE* fp;
	fp = fopen("dilation.bmp", "rb");
	if (fp == NULL) {
		printf("File not found!\n");
		return -1;
	}
	fread(&hf, sizeof(BITMAPFILEHEADER), 1, fp);
	fread(&hInfo, sizeof(BITMAPINFOHEADER), 1, fp);
	int ImgSize = hInfo.biWidth * hInfo.biHeight;
	int H = hInfo.biHeight, W = hInfo.biWidth;
	BYTE* Image;
	BYTE* Temp;
	BYTE* Output;

	if (hInfo.biBitCount == 24) { // 트루컬러
		Image = (BYTE*)malloc(ImgSize * 3);
		Temp = (BYTE*)malloc(ImgSize * 3);
		Output = (BYTE*)malloc(ImgSize * 3);
		fread(Image, sizeof(BYTE), ImgSize * 3, fp);	// 3색이므로 3배만큼 읽어온다
	}
	else { // 인덱스(그레이)
		fread(hRGB, sizeof(RGBQUAD), 256, fp);	// 트루컬러일 때는 팔레트가 없으므로 이부분이 없다
		Image = (BYTE*)malloc(ImgSize);
		Temp = (BYTE*)malloc(ImgSize);
		Output = (BYTE*)malloc(ImgSize);
		fread(Image, sizeof(BYTE), ImgSize, fp);
	}
	fclose(fp);
	
	int Histo[256] = { 0 };
	int AHisto[256] = { 0 };

	// 10주차
	//// (50, 40)위치를 특정 색상(R)으로 
	//FillColor(Image, 50, 40, W, H, 255, 0, 0);

	//// 높이 200인 가로 선
	//for (int i = 0; i < W; i++)
	//	FillColor(Image, i, 200, W, H, 0, 255, 255);	

	//// (50, 100) ~ (300, 400) 박스 채우기
	//for (int i = 100; i <= 400; i++) {
	//	for (int j = 50; j <= 300; j++) {
	//		FillColor(Image, j, i, W, H, 255, 0, 255);
	//	}
	//}

	//// 가로 띠 만들기 (R, G, B 일부씩 겹쳐서 가로 칠하기)
	//// RGB 모두 0으로 초기화
	//for (int i = 0; i <  H; i++) {
	//	for (int j = 0; j < W; j++) {
	//		Image[i * W * 3 + j * 3] = 0;		// B
	//		Image[i * W * 3 + j * 3 + 1] = 0;	// G
	//		Image[i * W * 3 + j * 3 + 2] = 0;	// R
	//	}
	//}
	//// y좌표 기준 0~239 (Red)
	//for (int i = 0; i < 240; i++) {
	//	for (int j = 0; j < W; j++) {
	//		Image[i * W * 3 + j * 3 + 2] = 255;	// Red 부분만 채운다
	//	}
	//}
	//// y좌표 기준 120 ~ 359 (Green)
	//for (int i = 120; i < 360; i++) {
	//	for (int j = 0; j < W; j++) {
	//		Image[i * W * 3 + j * 3 + 1] = 255;
	//	}
	//}
	//// y좌표 기준 240 ~ 479 (Blue)
	//for (int i = 240; i < 480; i++) {
	//	for (int j = 0; j < W; j++) {
	//		Image[i * W * 3 + j * 3] = 255;
	//	}
	//}

	//// 그라데이션 만들기 (Cyan ~ Red)
	//double wt;	// weight
	//for (int a = 0; a < 120; a++) {		// y좌표
	//	for (int i = 0; i < W; i++) {
	//		wt = i / (double)(W - 1);
	//		Image[a * W * 3 + i * 3] = (BYTE)(255 * (1.0 - wt));		// Blue
	//		Image[a * W * 3 + i * 3 + 1] = (BYTE)(255 * (1.0 - wt));	// Green
	//		Image[a * W * 3 + i * 3 + 2] = (BYTE)(255 * wt);			// Red
	//	}
	//}

	//// 10주차 실습과제
	//double weight;
	//// 초기화
	//for (int i = 0; i < H; i++) {
	//	for (int j = 0; j < W; j++) {
	//		Image[i * W * 3 + j * 3] = 0;		// B
	//		Image[i * W * 3 + j * 3 + 1] = 0;	// G
	//		Image[i * W * 3 + j * 3 + 2] = 0;	// R
	//	}
	//}

	//for (int a = 0; a < 160; a++) {	
	//	for (int i = 0; i < W; i++) {
	//		weight = i / (double)(W - 1);
	//		Image[a * W * 3 + i * 3] = (BYTE)(255 * (1.0 - weight));	// Blue
	//		Image[a * W * 3 + i * 3 + 1] = (BYTE)(255 * weight);		// Green
	//		Image[a * W * 3 + i * 3 + 2] = (BYTE)(255 * weight);		// Red
	//	}
	//}

	//for (int a = 160; a < 320; a++) {
	//	for (int i = 0; i < W; i++) {
	//		weight = i / (double)(W - 1);
	//		Image[a * W * 3 + i * 3] = (BYTE)(255 * weight);				// Blue
	//		Image[a * W * 3 + i * 3 + 1] = (BYTE)(255 * (1.0 - weight));	// Green
	//		Image[a * W * 3 + i * 3 + 2] = (BYTE)(255 * weight);			// Red
	//	}
	//}

	//for (int a = 320; a < 480; a++) {
	//	for (int i = 0; i < W; i++) {
	//		weight = i / (double)(W - 1);
	//		Image[a * W * 3 + i * 3] = (BYTE)(255 * weight);				// Blue
	//		Image[a * W * 3 + i * 3 + 1] = (BYTE)(255 * weight);			// Green
	//		Image[a * W * 3 + i * 3 + 2] = (BYTE)(255 * (1.0 - weight));	// Red
	//	}
	//}

	//// 10주차 학습활동
	//Binarization(Image, Temp, W, H, 100);
	//Dilation(Temp, Output, W, H);

	// 11주차
	////Red값이 큰 화소만 masking (R, G, B 모델 기준) -> 다른 R값 큰 영역도 같이 나타나서 딸기만 추출 실패
	//for (int i = 0; i < H; i++) { // Y좌표
	//	for (int j = 0; j < W; j++) { // X좌표
	//		if (Image[i * W * 3 + j * 3 + 2] > 200) {	//R값이 200보다 크면 딸기 영역으로 추측하므로 그대로 출력
	//			Output[i * W * 3 + j * 3] = Image[i * W * 3 + j * 3];	//B
	//			Output[i * W * 3 + j * 3 + 1] = Image[i * W * 3 + j * 3 + 1];	//G
	//			Output[i * W * 3 + j * 3 + 2] = Image[i * W * 3 + j * 3 + 2];	//R
	//		}
	//		else	// 딸기 영역이 아니면 0으로 채우기
	//			Output[i * W * 3 + j * 3] = Output[i * W * 3 + j * 3 + 1] = Output[i * W * 3 + j * 3 + 2] = 0;
	//	}
	//}

	////Red값이 큰 화소만 masking (R, G, B 모델 기준) -> G, B 조건도 추가하여 딸기만 추출
	//for (int i = 0; i < H; i++) { // Y좌표
	//	for (int j = 0; j < W; j++) { // X좌표
	//		if (Image[i * W * 3 + j * 3 + 2] > 130 &&	//R
	//			Image[i * W * 3 + j * 3 + 1] < 50 &&	//G
	//			Image[i * W * 3 + j * 3 + 0] < 100) {	//B
	//			Output[i * W * 3 + j * 3] = Image[i * W * 3 + j * 3];
	//			Output[i * W * 3 + j * 3 + 1] = Image[i * W * 3 + j * 3 + 1];
	//			Output[i * W * 3 + j * 3 + 2] = Image[i * W * 3 + j * 3 + 2];
	//		}
	//		else
	//			Output[i * W * 3 + j * 3] = Output[i * W * 3 + j * 3 +1] = Output[i * W * 3 + j * 3 +2] = 0;
	//	}
	//}

	//// 빨간색 딸기영역만 masking (Y, Cb, Cr 모델 기준)
	//BYTE* Y = (BYTE *)malloc(ImgSize);
	//BYTE* Cb = (BYTE*)malloc(ImgSize);
	//BYTE* Cr = (BYTE*)malloc(ImgSize);

	//RGB2YCbCr(Image, Y, Cb, Cr, W, H);

	////// Y, Cb, Cr 이미지 확인
	////fp = fopen("Y.raw", "wb");
	////fwrite(Y, sizeof(BYTE), W* H, fp);	
	////fclose(fp);
	////fp = fopen("Cb.raw", "wb");
	////fwrite(Cb, sizeof(BYTE), W* H, fp);
	////fclose(fp);
	////fp = fopen("Cr.raw", "wb");
	////fwrite(Cr, sizeof(BYTE), W* H, fp);	
	////fclose(fp);

	//for (int i = 0; i < H; i++) {
	//	for (int j = 0; j < W; j++) {
	//		if (Cb[i * W + j] < 140 && Cr[i * W + j] > 190) {	// 빨강 역역이면 그대로 출력
	//			Output[i * W * 3 + j * 3] = Image[i * W * 3 + j * 3];
	//			Output[i * W * 3 + j * 3 + 1] = Image[i * W * 3 + j * 3 + 1];
	//			Output[i * W * 3 + j * 3 + 2] = Image[i * W * 3 + j * 3 + 2];
	//		}
	//		else	// 아니면 검정으로 출력
	//			Output[i * W * 3 + j * 3] = Output[i * W * 3 + j * 3 + 1] = Output[i * W * 3 + j * 3 + 2] = 0;
	//	}
	//}

	//free(Y);
	//free(Cb);
	//free(Cr);

	//// 11주차 실습과제
	//BYTE* Y = (BYTE *)malloc(ImgSize);
	//BYTE* Cb = (BYTE*)malloc(ImgSize);
	//BYTE* Cr = (BYTE*)malloc(ImgSize);

	//RGB2YCbCr(Image, Y, Cb, Cr, W, H);

	//for (int i = 0; i < H; i++) {
	//	for (int j = 0; j < W; j++) {
	//		if (77 < Cb[i * W + j] && Cb[i * W + j] < 127 && 133 < Cr[i * W + j] && Cr[i * W + j] < 173) {
	//			Output[i * W * 3 + j * 3] = Image[i * W * 3 + j * 3];
	//			Output[i * W * 3 + j * 3 + 1] = Image[i * W * 3 + j * 3 + 1];
	//			Output[i * W * 3 + j * 3 + 2] = Image[i * W * 3 + j * 3 + 2];
	//		}
	//		else	// 아니면 검정으로 출력
	//			Output[i * W * 3 + j * 3] = Output[i * W * 3 + j * 3 + 1] = Output[i * W * 3 + j * 3 + 2] = 0;
	//	}
	//}

	//// 사각형 그릴 좌표 계산
	//int LU_X = W - 1, RD_X = 0, LU_Y = H - 1, RD_Y = 0;
	//for (int i = 0; i < H; i++)			// Output 배열 전체 순회
	//{
	//	for (int j = 0; j < W; j++)
	//	{
	//		if (Output[i * W * 3 + j * 3] != 0 && 
	//			Output[i * W * 3 + j * 3 + 1] != 0 && 
	//			Output[i * W * 3 + j * 3 + 2] != 0)	// 경계일 경우
	//		{
	//			if (j < LU_X) LU_X = j;
	//			if (j > RD_X) RD_X = j;
	//			if (i < LU_Y) LU_Y = i;
	//			if (i > RD_Y) RD_Y = i;
	//		}
	//	}
	//}

	//// 피부영역 빨간색 사각형 그리기
	//for (int i = 0; i < H; i++)
	//{
	//	for (int j = 0; j < W; j++)
	//	{
	//		if (LU_X <= j && j <= RD_X && LU_Y <= i && i <= RD_Y)		// 직사각형 범위 내 이면서
	//		{
	//			if (j == LU_X || j == RD_X || i == LU_Y || i == RD_Y)	// 접선인 경우
	//				Output[i * W * 3 + j * 3 + 2] = 255;
	//		}
	//	}
	//}

	//free(Y);
	//free(Cb);
	//free(Cr);

	//12주차 : 모폴로지 연산(침식, 팽창)

	//// erosion.bmp 7회 침식 
	//Erosion(Image, Output, W, H);
	//Erosion(Output, Image, W, H);
	//Erosion(Image, Output, W, H);
	//Erosion(Output, Image, W, H);
	//Erosion(Image, Output, W, H);
	//Erosion(Output, Image, W, H);
	//Erosion(Image, Output, W, H);

	//// dilation.bmp 3회 팽창, 2회 침식 (닫힘 연산) 후 세선화
	//Dilation(Image, Output, W, H);
	//Dilation(Output, Image, W, H);
	//Dilation(Image, Output, W, H);
	//Erosion(Output, Image, W, H);
	//Erosion(Image, Output, W, H);
	//Erosion(Output, Image, W, H);	// 전경 255, 배경 0	
	//InverseImage(Image, Image, W, H); // zhangSuen() 의 전경이 0, 배경이 255 이므로 반전해주기
	//zhangSuen(Image, Output, H, W);

	//12주차 실습과제 : 세선화 후 분기점 및 끝점 검출 및 표시
	Dilation(Image, Output, W, H);
	Dilation(Output, Image, W, H);
	Dilation(Image, Output, W, H);
	Erosion(Output, Image, W, H);
	Erosion(Image, Output, W, H);
	Erosion(Output, Image, W, H);	
	InverseImage(Image, Image, W, H);
	zhangSuen(Image, Temp, H, W);
	FeatureExtractThinImage(Temp, Output, W, H);

	SaveBMPFile(hf, hInfo, hRGB, Output, hInfo.biWidth, hInfo.biHeight, "output2.bmp");

	free(Image);
	free(Temp);
	free(Output);
	return 0;
}