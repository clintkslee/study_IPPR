#pragma warning(disable:4996)
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>

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



int main()
{
	BITMAPFILEHEADER hf;		// 14바이트
	BITMAPINFOHEADER hInfo;		// 40바이트
	RGBQUAD hRGB[256];			// 1024바이트
	FILE* fp;
	fp = fopen("coin.bmp", "rb");
	if (fp == NULL) {
		printf("File not found!\n");
		return -1;
	}

	fread(&hf, sizeof(BITMAPFILEHEADER), 1, fp);
	fread(&hInfo, sizeof(BITMAPINFOHEADER), 1, fp);
	fread(hRGB, sizeof(RGBQUAD), 256, fp);
	int ImgSize = hInfo.biWidth * hInfo.biHeight;
	BYTE * Image = (BYTE *)malloc(ImgSize);
	BYTE * Output = (BYTE*)malloc(ImgSize);
	fread(Image, sizeof(BYTE), ImgSize, fp);
	fclose(fp);

	/* 영상 처리 부분 시작 */

	//영상 반전 
	//InverseImage(Image, Output, hInfo.biWidth, hInfo.biHeight);

	// 영상 밝기 조절
	//BrightnessAdj(Image, Output, hInfo.biWidth, hInfo.biHeight, 70);

	// 영상 대비 조절
	//ContrastAdj(Image, Output, hInfo.biWidth, hInfo.biHeight, 0.5);

	// 히스토그램 구하기
	int Histo[256] = { 0 };	// 영상의 히스토그램 
	ObtainHistogram(Image, Histo, hInfo.biWidth, hInfo.biHeight);
	
	// 누적 히스토그램 구하기
	//int AHisto[256] = { 0 }; // 영상의 누적히스토그램
	//ObtainAHistogram(Histo, AHisto);

	// 히스토그램 스트레칭
	//HistogramStretching(Image, Output, Histo, hInfo.biWidth, hInfo.biHeight);
	
	// 히스토그램 평활화
	//HistogramEqualization(Image, Output, AHisto, hInfo.biWidth, hInfo.biHeight);

	// 영상 이진화
	int Thres = GozalezBinThresh(Histo); // 임계값 지정(Gonzalez, Woods 방법)
	Binarization(Image, Output, hInfo.biWidth, hInfo.biHeight, Thres);
	

	/* 영상 처리 부분 끝 */

	fp = fopen("output.bmp", "wb");
	fwrite(&hf, sizeof(BYTE), sizeof(BITMAPFILEHEADER), fp);
	fwrite(&hInfo, sizeof(BYTE), sizeof(BITMAPINFOHEADER), fp);
	fwrite(hRGB, sizeof(RGBQUAD), 256, fp);
	fwrite(Output, sizeof(BYTE), ImgSize, fp);
	fclose(fp);
	free(Image);
	free(Output);
	return 0;
}