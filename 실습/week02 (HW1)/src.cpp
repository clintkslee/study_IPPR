#pragma warning(disable:4996)
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>

void SaveBMPFile(BITMAPFILEHEADER hf, BITMAPINFOHEADER hInfo, RGBQUAD* hRGB, BYTE* Output, int W, int H, const char* FileName)
{
	FILE* fp = fopen(FileName, "wb");
	fwrite(&hf, sizeof(BYTE), sizeof(BITMAPFILEHEADER), fp);
	fwrite(&hInfo, sizeof(BYTE), sizeof(BITMAPINFOHEADER), fp);
	fwrite(hRGB, sizeof(RGBQUAD), 256, fp);
	fwrite(Output, sizeof(BYTE), W*H, fp);
	fclose(fp);
}

int main()
{
	BITMAPFILEHEADER hf;	// BMP 파일헤더 14Bytes
	BITMAPINFOHEADER hInfo; // BMP 인포헤더 40Bytes
	RGBQUAD hRGB[256];		// 팔레트 (256 * 4Bytes)
	FILE* fp;

	fp = fopen("lenna.bmp", "rb");
	if(fp == NULL) 
	{
		printf("File not found!\n");
		return -1;
	}

	/* 입력 */

	fread(&hf, sizeof(BITMAPFILEHEADER), 1, fp);
	fread(&hInfo, sizeof(BITMAPINFOHEADER), 1, fp);
	fread(hRGB, sizeof(RGBQUAD), 256, fp);
	int ImgSize = hInfo.biWidth * hInfo.biHeight;

	BYTE* Image = (BYTE*)malloc(ImgSize);		// 원본 영상 담을 배열
	fread(Image, sizeof(BYTE), ImgSize, fp);
	fclose(fp);

	BYTE* Output = (BYTE*)malloc(ImgSize);		// 출력 영상 담을 배열
	
	/* 영상처리 */
	
	//output1.bmp: lenna.bmp 파일과 동일한 영상
	SaveBMPFile(hf, hInfo, hRGB, Image, hInfo.biWidth, hInfo.biHeight, "output1.bmp");

	//output2.bmp: 원본 영상의 밝기값을 50만큼 증가시킨 영상 (별도 overflow로 인한 클리핑처리는 필요없음)
	for (int i = 0; i < ImgSize; i++)
			Output[i] = Image[i] + 50;
	SaveBMPFile(hf, hInfo, hRGB, Output, hInfo.biWidth, hInfo.biHeight, "output2.bmp");

	//output3.bmp: 원본 영상의 반전 결과 영상(negative image)
	for (int i = 0; i < ImgSize; i++)
		Output[i] = 255 - Image[i];
	SaveBMPFile(hf, hInfo, hRGB, Output, hInfo.biWidth, hInfo.biHeight, "output3.bmp");


	free(Output);
	free(Image);
	return 0;
}