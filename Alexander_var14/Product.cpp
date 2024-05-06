#define _CRT_SECURE_NO_DEPRECATE
#include <stdint.h>
#include <stdlib.h>
#include <locale.h>
#include <stdio.h>

int argv_check(char* argv)
{
	int j = 0, i = 0;
	while (true)
	{
		if (argv[i] == '.')
		{
			j += 1;
			if (argv[i + 1] == 't')
			{
				if (argv[i + 2] == 'x')
				{
					if (argv[i + 3] == 't')
					{
						return 1;
					}
					else
					{
						return 0;
					}
				}
				else
				{
					return 0;
				}
			}
			else
			{
				return 0;
			}
		}
		i++;
	}
}

int str_lenght(FILE* f)
{
	uint8_t sym;
	int len = 0;
	int pos = ftell(f);
	while (true)
	{
		sym = fgetc(f);
		len++;
		if ((sym & 224) == 192)
		{
			sym = fgetc(f);
		}
		if (sym == 10||sym == 255||sym == 13)
		{
			len--;
			break;
		}
	}
	fseek(f, pos, SEEK_SET);
	return len;
}

int file_lenght(FILE* f)	
{
	uint8_t  sym;
	int len = 0;
	int pos = ftell(f);
	while (!feof(f))
	{
		sym = fgetc(f);
		len++;
		if ((sym & 224) == 192)
		{
			sym = fgetc(f);
		}
	}
	fseek(f, pos, SEEK_SET);
	return len;
}

char UTF8To1251(FILE* f)
{
	uint8_t sym1, sym = fgetc(f);
	if ((sym & 224) == 192)
	{
		if ((sym == 208) || (sym == 209))
		{
			sym1 = fgetc(f);
			if (sym1 >= 128 && sym1 <= 143)
			{
				sym1 += 112;
				return sym1;
			}
			else if (sym1 >= 144 && sym1 <= 191)
			{
				sym1 += 48;
				return sym1;
			}
		}
	}
	return sym;
}

int decode_str_len(FILE* f)
{
	uint8_t sym;
	int len = 0;
	int pos = ftell(f);
	while (true)
	{
		sym = fgetc(f);
		len++;
		if ((sym & 224) == 192)
		{
			sym = fgetc(f);
		}
		else if (sym == '=')
		{
			len--;
		}
		if (sym == 10 || sym == 255 || sym == 13)
		{
			len--;
			break;
		}
	}
	fseek(f, pos, SEEK_SET);
	return len;
}

void encode_BASE64(FILE* file, FILE* out_file,int file_ln, unsigned char arr[])
{
	unsigned char char_base64, c_inp;
	while (file_ln)
	{
		int l = 0;
		int str_len = str_lenght(file);
		while (str_len)
		{
			if (str_len == 1)
			{
				c_inp = UTF8To1251(file);
				char_base64 = c_inp >> 2;
				fprintf(out_file, "%c", arr[char_base64]);
				char_base64 = (c_inp << 4) & 0x30;
				fprintf(out_file, "%c", arr[char_base64]);
				l += 2;
				str_len -= 1;
				file_ln -= 1;
				continue;
			}
			else if (str_len == 2)
			{
				c_inp = UTF8To1251(file);	
				char_base64 = c_inp >> 2;
				fprintf(out_file, "%c", arr[char_base64]);
				char_base64 = (c_inp << 4) & 0x30;
				c_inp = UTF8To1251(file);
				char_base64 |= (c_inp >> 4);
				fprintf(out_file, "%c", arr[char_base64]);
				char_base64 = (c_inp << 2) & 0x3C;
				fprintf(out_file, "%c", arr[char_base64]);
				l += 3;
				str_len -= 2;
				file_ln -= 2;
				continue;
			}
			else if (str_len >= 3)
			{
				c_inp = UTF8To1251(file);
				char_base64 = c_inp >> 2;
				fprintf(out_file, "%c", arr[char_base64]);
				char_base64 = (c_inp << 4) & 0x30;
				c_inp = UTF8To1251(file);
				char_base64 |= (c_inp >> 4);
				fprintf(out_file, "%c", arr[char_base64]);
				char_base64 = (c_inp << 2) & 0x3C;
				c_inp = UTF8To1251(file);
				char_base64 |= c_inp >> 6;
				fprintf(out_file, "%c", arr[char_base64]);
				char_base64 = c_inp & 0x3F;
				fprintf(out_file, "%c", arr[char_base64]);
				l += 4;
				str_len -= 3;
				file_ln -= 3;
				continue;
			}
		}
		while (l % 4 != 0)
		{
			fprintf(out_file, "=");
			l++;
		}
		if (str_len == 0)
		{
			c_inp = UTF8To1251(file);
			if (c_inp == 13)
			{
				fprintf(out_file, "%c", c_inp);
				c_inp = UTF8To1251(file);
				fprintf(out_file, "%c", c_inp);
				file_ln -= 2;
			}
			else if (c_inp == 10 || c_inp == 255)
			{
				fprintf(out_file, "%c", 10);
				file_ln -= 1;
			}
		}
	}
	fclose(file);
	fclose(out_file);
}

void decode_BASE64(FILE* file, FILE* out_file, int file_ln, unsigned char arr[])
{
	int index;
	uint8_t char_1251, c_inp;
	while (file_ln)
	{
		int dstr_len = decode_str_len(file);
		int str_len = str_lenght(file);
		while (dstr_len)
		{
			if (dstr_len == 2)
			{
				c_inp = fgetc(file);
				for (index = 0; index < 64; index++)
				{
					if (c_inp == arr[index])
					{
						char_1251 = index << 2;	
						c_inp = fgetc(file);
						for (index = 0; index < 64; index++)
						{
							if (c_inp == arr[index])
							{
								char_1251 |= index >> 4;
								fprintf(out_file, "%c", char_1251);
							}
						}
					}
				}
				file_ln -= 2;
				str_len -= 2;
				dstr_len -= 2;
			}
			else if (dstr_len == 3)
			{
				c_inp = fgetc(file);
				for (index = 0; index < 64; index++)
				{
					if (c_inp == arr[index])
					{
						char_1251 = index << 2;
						c_inp = fgetc(file);
						for (index = 0; index < 64; index++)
						{
							if (c_inp == arr[index])
							{
								char_1251 |= index >> 4;
								fprintf(out_file, "%c", char_1251);
								char_1251 = (index & 0xF) << 4;
								c_inp = fgetc(file);
								for (index = 0; index < 64; index++)
								{
									if (c_inp == arr[index])
									{
										char_1251 |= ((index & 0x3C) >> 2);
										fprintf(out_file, "%c", char_1251);
									}
								}
							}
						}
					}
				}
				file_ln -= 3;
				str_len -= 3;
				dstr_len -= 3;
			}
			else if (dstr_len >= 4)
			{
				c_inp = fgetc(file);
				for (index = 0; index < 64; index++)
				{
					if (c_inp == arr[index])
					{
						char_1251 = index << 2;
						c_inp = fgetc(file);
						for (index = 0; index < 64; index++)
						{
							if (c_inp == arr[index])
							{
								char_1251 |= index >> 4;
								fprintf(out_file, "%c", char_1251);
								char_1251 = (index & 0xF) << 4;
								c_inp = fgetc(file);	
								for (index = 0; index < 64; index++)
								{
									if (c_inp == arr[index])
									{
										char_1251 |= ((index & 0x3C) >> 2);
										fprintf(out_file, "%c", char_1251);
										char_1251 = (index & 0x3) << 6;
										c_inp = fgetc(file);
										for (index = 0; index < 64; index++)
										{
											if (c_inp == arr[index])
											{
												char_1251 |= index;
												fprintf(out_file, "%c", char_1251);
											}
										}
									}
								}
							}
						}
					}
				}
				file_ln -= 4;
				str_len -= 4;
				dstr_len -= 4;
			}
		}
		while (dstr_len != str_len)
		{
			c_inp = fgetc(file);
			file_ln--;
			str_len--;
		}	
		c_inp = UTF8To1251(file);
		if (c_inp == 13)
		{
			fprintf(out_file, "%c", c_inp);
			c_inp = UTF8To1251(file);
			fprintf(out_file, "%c", c_inp);
			file_ln -= 2;
		}
		else if (c_inp == 10 || c_inp == 255)
		{
			fprintf(out_file, "%c", 10);
			file_ln -= 1;
		}
	}
	fclose(file);
	fclose(out_file);
}

int main(int argc, char* argv[])
{
	int type = atoi(argv[1]);
	setlocale(LC_CTYPE, "");
	if (type == 0 || type == 1)
	{
		if (int exists = argv_check(argv[2]) == 0)
		{
			printf("\nПроверьте введенное название исходного файла");
			printf("\nНАПОМИНАНИЕ:");
			printf("\nВведенное значение должно сожержать формат текстового документа. НАПРИМЕР: 123.txt\n");
			if (int exists = argv_check(argv[3]) == 0)
			{
				printf("\nПроверьте введенное название выходного файла");
				printf("\nНАПОМИНАНИЕ:");
				printf("\nВведенное значение должно сожержать формат текстового документа. НАПРИМЕР: 321.txt\n");
			}
			return 0;
		}
		if (int exists = argv_check(argv[3]) == 0)
		{
			printf("\nПроверьте введенное название выходного файла");
			printf("\nНАПОМИНАНИЕ:");
			printf("\nВведенное значение должно сожержать формат текстового документа. НАПРИМЕР: 321.txt\n");
			return 0;
		}
	}
	else
	{
		printf("\nПроверьте правильность ввода режима программы");
		printf("\nНАПОМИНАНИЕ:");
		printf("\nПрограмма работает в двух режимах(encode(1) и decode(0))\n");
		if (int exists = argv_check(argv[2]) == 0)
		{
			printf("\nПроверьте введенное название исходного файла");
			printf("\nНАПОМИНАНИЕ:");
			printf("\nВведенное значение должно сожержать формат текстового документа. НАПРИМЕР: 123.txt\n");
		}
		if (int exists = argv_check(argv[3]) == 0)
		{
			printf("\nПроверьте введенное название выходного файла");
			printf("\nНАПОМИНАНИЕ:");
			printf("\nВведенное значение должно сожержать формат текстового документа. НАПРИМЕР: 321.txt\n");
		}
		return 0;
	}
	FILE* file = fopen(argv[2], "rb");
	FILE* out_file = fopen(argv[3], "wb");
	unsigned char arr[] = { 'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
		'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z',
		'0','1','2','3','4','5','6','7','8','9','+','/'};
	int file_ln = file_lenght(file);
	if (type == 1)
	{
		encode_BASE64(file, out_file, file_ln, arr);
	}
	else if (type == 0)
	{
		decode_BASE64(file,out_file,file_ln, arr);
	}
	printf("\n----END---\n");
	return 0;
}