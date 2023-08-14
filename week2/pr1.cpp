#pragma warning(disable:4996)
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
int main()
{
	BITMAPFILEHEADER hf;	// BMP 파일헤더 14Bytes
	BITMAPINFOHEADER hInfo; // BMP 인포헤더 40Bytes
	RGBQUAD hRGB[256];		// 팔레트 (256 * 4Bytes)
	FILE* fp;

	fp = fopen("lenna.bmp", "rb");
	if(fp == NULL) return 0;

	/* 입력 */

	fread(&hf, sizeof(BITMAPFILEHEADER), 1, fp);
	fread(&hInfo, sizeof(BITMAPINFOHEADER), 1, fp);
	fread(hRGB, sizeof(RGBQUAD), 256, fp);
	int ImgSize = hInfo.biWidth * hInfo.biHeight;

	BYTE* Image = (BYTE*)malloc(ImgSize);
	fread(Image, sizeof(BYTE), ImgSize, fp);
	fclose(fp);

	BYTE* Output1 = (BYTE*)malloc(ImgSize);
	BYTE* Output2 = (BYTE*)malloc(ImgSize);
	BYTE* Output3 = (BYTE*)malloc(ImgSize);
	
	/* 영상처리 */
	
	//output1.bmp: lenna.bmp 파일과 동일한 영상
	for(int i=0; i<ImgSize; i++)
		Output1[i] = Image[i];

	//output2.bmp: 원본 영상의 밝기값을 50만큼 증가시킨 영상 (별도 overflow로 인한 클리핑처리는 필요없음)
	for (int i = 0; i < ImgSize; i++)
		Output2[i] = Image[i] + 50;

	//output3.bmp: 원본 영상의 반전 결과 영상(negative image)
	for (int i = 0; i < ImgSize; i++)
		Output3[i] = 255 - Image[i];

	/* 출력 */

	fp = fopen("output1.bmp", "wb");
	fwrite(&hf, sizeof(BYTE), sizeof(BITMAPFILEHEADER), fp);
	fwrite(&hInfo, sizeof(BYTE), sizeof(BITMAPINFOHEADER), fp);
	fwrite(hRGB, sizeof(RGBQUAD), 256, fp);
	fwrite(Output1, sizeof(BYTE), ImgSize, fp);
	fclose(fp);
	free(Output1);

	fp = fopen("output2.bmp", "wb");
	fwrite(&hf, sizeof(BYTE), sizeof(BITMAPFILEHEADER), fp);
	fwrite(&hInfo, sizeof(BYTE), sizeof(BITMAPINFOHEADER), fp);
	fwrite(hRGB, sizeof(RGBQUAD), 256, fp);
	fwrite(Output2, sizeof(BYTE), ImgSize, fp);
	fclose(fp);
	free(Output2);

	fp = fopen("output3.bmp", "wb");
	fwrite(&hf, sizeof(BYTE), sizeof(BITMAPFILEHEADER), fp);
	fwrite(&hInfo, sizeof(BYTE), sizeof(BITMAPINFOHEADER), fp);
	fwrite(hRGB, sizeof(RGBQUAD), 256, fp);
	fwrite(Output3, sizeof(BYTE), ImgSize, fp);
	fclose(fp);
	free(Output3);

	free(Image);

	return 0;
}