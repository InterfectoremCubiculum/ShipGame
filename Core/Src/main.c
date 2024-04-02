/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "lcd.h"
#include "math.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
UART_HandleTypeDef huart2;
double VSelect = 2.6;
double VLeft = 1.75;
double VDown = 1.1;
double VUp = 0.4;
double VRight = 0;


/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

// WYPISYWANIE TEKSTU NA UART
int __io_putchar(int ch)
{
    if (ch == '\n') {
        uint8_t ch2 = '\r';
        HAL_UART_Transmit(&huart2, &ch2, 1, HAL_MAX_DELAY);
    }
    HAL_UART_Transmit(&huart2, (uint8_t*)&ch, 1, HAL_MAX_DELAY);
    return 1;
}

//POBIERANIE TEKSTU Z UART
int __io_getchar(void) {
    uint8_t ch;
        HAL_UART_Receive(&huart2, &ch, 1, HAL_MAX_DELAY);
        if (ch == '\r\n') {
            HAL_UART_Receive(&huart2, &ch, 1, HAL_MAX_DELAY);
        }
        HAL_UART_Transmit(&huart2, &ch, 1, HAL_MAX_DELAY);
        return ch;
}

//POBIERA DANE WYJSCIOWE ORAZ WSTAWIA JE DO BUFFORA
int _read( char *buffor, int buffor_size)
{
    int count = 0;
    char ch;
    while (1)
    {
        ch = __io_getchar();
        if (ch == '\r')
        {
            break;
        }
        buffor[count++] = ch;
    }
    buffor[count] = '\0';
    if(count >= buffor_size){
    	count =0;
    }
    return count; // Zwraca 0 gdy buffor jest przepelniony
}

// WLASNE ZNAKI STATKI I PUDLA
uint8_t customChar[7][8] = {
		{		  0b00000,
				  0b00000,
				  0b00000,
				  0b00000,
				  0b00100,
				  0b00110,
				  0b10101,
				  0b01110}, //STATEK DOL 0X00

		{		  0b00100,
				  0b00110,
				  0b10101,
				  0b01110,
				  0b00000,
				  0b00000,
				  0b00000,
				  0b00000}, //STATEK GORA 0X01

		{		  0b00000,
				  0b00000,
				  0b00000,
				  0b00000,
				  0b00000,
				  0b01110,
				  0b01110,
				  0b00000}, //PUDLO DOL 0X02

		{		  0b00000,
				  0b01110,
				  0b01110,
				  0b00000,
				  0b00000,
				  0b00000,
				  0b00000,
				  0b00000}, //PUDLO GORA 0X03

		{		  0b00000,
				  0b01110,
				  0b01110,
				  0b00000,
				  0b00100,
				  0b00110,
				  0b10101,
				  0b01110}, //PUDLO GORA - STATEK DOL 0X04

		{		  0b00100,
				  0b00110,
				  0b10101,
				  0b01110,
				  0b00000,
				  0b01110,
				  0b01110,
				  0b00000}, //STATEK GORA - PODLO DOL 0X05

		{		  0b00000,
				  0b01110,
				  0b01110,
				  0b00000,
				  0b00000,
				  0b01110,
				  0b01110,
				  0b00000}, //PUDLO GORA - PUDLO DOL 0X06
};

// FUNKCJA TWORZACA WLASNE ZNAKI
void Create_char(uint8_t start_location, uint8_t char_table[][8], uint8_t num_chars)
{
    for (uint8_t j = 0; j < num_chars; j++) {
        uint8_t location = start_location + j;
        lcd_cmd(0x40 | (location << 3));
        for (int i = 0; i < 8; i++)
        {
            lcd_char_cp(char_table[j][i]);
        }
    }
}

//Sprawdza w ktorej linii znajduje sie obiekt, zwraca w ktorej linii
int Check_line(int y)
{
	if(y>=3)
	{
		return 2;
	}
	else
	{
		return 1;
	}
}

int round_zero = 0; //Czy nie jest to zerowa runda
int position_y = 1; // Gdzie znajduje sie statek w poziomie
int position_x = 1; // Gdzie znajduje sie statek w pionie
int miss_in = 0; //Czy przy przemieszczaniu sie na wczesniejszej pozycji bylo pudlo nad statkiem (4) lub pod statkiem (5)

//WYPISUJE STATEK W DANEJ LINII 1/2 ORAZ W KTOREJ POLOWIE LINII GORNEJ/DOLNEJ
void Print_in_lcd()
{
	int line = Check_line(position_y);
	if(!round_zero) // Jezeli nie jest runda zerowa to czysci ekran
	{
		lcd_clear();
	}
	if(position_y%2==0)
	{
	    lcd_char(line,position_x,0x00);
	}
	else
	{
	    lcd_char(line,position_x,0x01);
	}
	HAL_Delay(500);
}

//USTALENIE LINII I CZYSZCZENIE POPRZEDNIEJ POZYCJI STATKU W RUNDZIE NIEZEROWEJ (POMOCNE W FUNKCJI Keyboard())
void Positioning(){
	int line = Check_line(position_y);
	if(miss_in == 4)
	{
		lcd_char(line,position_x,0x03);
	}
	else if(miss_in == 5)
	{
		lcd_char(line,position_x,0x02);
	}
	else
	{
		lcd_print(line,position_x," ");
	}
	miss_in = 0;
}

// TABLICA NA WSZYSTKO CO JEST NA EKRANIE
int tab[4][16];
void Init_tab(){
	for(int i=0 ; i<4 ; i++){
		for(int j=0 ; j<16 ; j++){
				// 0 - NIC
				// 1 - STATEK
				// 2 - PUDLO
				tab[i][j]=0;
			}
	}
}
// WYSPISUJE W UART CALA TAB
void Print_tab()
{
	printf(" ");
	for(int i=0 ; i<16 ; i++)
	{
		printf("%3d", i+1);
	}
	printf("\n");
    for(int i=0 ; i<4 ; i++)
    {
    	printf("%d", i+1);
        for(int j=0 ; j<16 ; j++)
        {
            // JEZELI TAB TO 0 - CZYLI NIC LUB 1 CZYLI STATEK WYPISUJEMY 0
            // JEZELEI TAB TO 2 CZYLI PUDLO TO X
        	if(tab[i][j]==0 || tab[i][j]==1)
            {
                printf("%3c", '-');
            }
            else if(tab[i][j]==2)
            {
                printf("%3c", 'X');
            }
        }
        printf("\n");
    }
}

// SPRAWDZA NAPIECIE Z PRZYCISKOW NA LCD
float Give_voltage()
{
	HAL_ADC_Start(&hadc1);
	HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
	HAL_ADC_Stop(&hadc1);
	uint32_t val = HAL_ADC_GetValue(&hadc1);
	float volt = (float)val * 3.3 / (pow(2,12)-1);
	return volt;
}

// SWOBODNE PORUSZANIE SIE STATKIEM W RUNDZIE ZEROWEJ
int Start_game_keyboard()
{
	 float volt = Give_voltage();
	 if(volt > VSelect && volt < 2.9) // SELECT
	 {
	    	return 3;
	 }
	 else if(volt > VLeft && volt < (VSelect - 0.2) && position_x>1 ) // LEFT
	 {
	    --position_x;
	    Print_in_lcd();
	 }
	 else if(volt > VDown && volt < (VLeft - 0.2) && position_y<4) // DOWN
	 {
		 ++position_y;
	     Print_in_lcd();
	 }
	 else if(volt > VUp && volt < (VDown - 0.2) && position_y>1 ) // UP
	 {

	     --position_y;
	     Print_in_lcd();
	 }
	 else if(volt < (VUp - 0.2)  && position_x<16) // RIGHT
	 {
		 ++position_x;
	     Print_in_lcd();
	 }
	 HAL_Delay(100);
	 return 0;
}
// WYBIERA MIEJSCE NA STATEK
int Keyboard()
{
	int line = Check_line(position_y);
    if(tab[position_y][position_x-1] == 2 && position_y%2==1)
    {
    	miss_in = 5;
    }
    else if(tab[position_y-2][position_x-1] == 2 && position_y%2==0)
    {
    	miss_in = 4;
    }

    float volt = Give_voltage();
    if(volt > VSelect && volt < VSelect) // SELECT
    {
    	return 3;
    }
    else if(volt > VLeft && volt < (VSelect - 0.2) && position_x>1 && tab[position_y-1][position_x-2] != 2) // LEFT
    {
    	Positioning();
    	--position_x;
    	if(tab[position_y-2][position_x-1] == 2 && position_y%2==0)
    	{
    		lcd_char(line,position_x,0x04);
    		HAL_Delay(500);
    	}
    	else if(tab[position_y][position_x-1] == 2 && position_y%2==1)
    	{
    		lcd_char(line,position_x,0x05);
    		HAL_Delay(500);
    	}
    	else
    	{
    		Print_in_lcd();
    	}
    }
    else if(volt > VDown && volt < (VLeft - 0.2) && position_y<4 && tab[position_y][position_x-1] != 2) // DOWN
    {
    	Positioning();
        ++position_y;
        if(tab[position_y][position_x-1] == 2 && position_y==3)
        {
        	lcd_char(line+1,position_x,0x05);
        	HAL_Delay(500);
        }
        else
        {
        	Print_in_lcd();
        }
    }
    else if(volt > VUp && volt < (VDown - 0.2)   && position_y>1 && tab[position_y-2][position_x-1] != 2) // UP
    {
    	Positioning();
        --position_y;
        if(tab[position_y-2][position_x-1] == 2 && position_y==2)
        {
        	lcd_char(line-1,position_x,0x04);
        	HAL_Delay(500);
        }
        else
        {
        	Print_in_lcd();
        }
    }
    else if(volt < (VUp - 0.2)   && position_x<16 && tab[position_y-1][position_x] != 2) // RIGHT
    {
    	Positioning();
    	++position_x;
    	if(tab[position_y-2][position_x-1] == 2 && position_y%2==0)
    	{
    		lcd_char(line,position_x,0x04);
    		HAL_Delay(500);
    	}
    	else if(tab[position_y][position_x-1] == 2 && position_y%2==1)
    	{
    		lcd_char(line,position_x,0x05);
    		HAL_Delay(500);
    	}
    	else
    	{
    		Print_in_lcd();
    	}
    }
    else
    {
    	return 0;
    }
	return 1;
}

int game = 1;
// W MENU SPRAWDZAMY CZY WCISNIETO SELECT PRZY STARCIE GRY
int Wait_for_select()
{
	float volt = Give_voltage();
	if(volt > VSelect && volt < 2.9) // SELECT
	{
		if(game==1)
		{
			lcd_clear();
			lcd_print(1, 1, "GAME 1 OF 2");
			lcd_print(2, 1, "P1 LCD - P2 UART");
			printf("GAME 1 OF 2 \nP1 LCD - P2 UART \r\n");
			HAL_Delay(1000);
			return 0;
		}
		else if(game==2)
		{
			lcd_clear();
			lcd_print(1, 1, "GAME 2 OF 2");
			lcd_print(2, 1, "P1 UART - P2 LCD");
			printf("GAME 2 OF 2 \nP1 UART - P2 LCD \r\n");
			HAL_Delay(1000);
			return 0;
		}
	}
	else
	{
		return 1;
	}
}

// ODPOWIADA ZA LICZBE RUCHOW GRACZA NA EKRANIE LCD
void LCD_player_move()
{
	int Selected = Keyboard();
	if(!round_zero)
	{
		lcd_char(1,position_x,0x01);
		while(Selected < 3)
		{
				Selected = Start_game_keyboard();
		}
		round_zero=1;
	}
	else
	{
		while(Selected <3)
		{
			Selected += Keyboard();
		}
	}
}

// WYPISUJE ZNAKI PO STRZALE NA EKRANIE LCD
void Change_lcd(int y, int x)
{

	int line = Check_line(y);
	if(y%2==0)
	{
		if(tab[y-2][x-1] == 2)
		{
			lcd_char(line,x,0x06);
		}
		else if(tab[y-2][x-1] == 1)
		{
			lcd_char(line,x,0x05);
		}
		else
		{
			lcd_char(line,x,0x02);
		}
	}
	else
	{
		if(tab[y][x-1] == 2){
			lcd_char(line,x,0x06);
		}
		else if(tab[y][x-1] == 1){
			lcd_char(line,x,0x04);
		}
		else{
			lcd_char(line,x,0x03);
		}

	}
	tab[y-1][x-1]=2;
}

int win = 0; //CZY TRAFIONO STATEK 0 - NIE 1 - TAK
int shoot_counter = 0; // ZLICZA LICZBE STRZALOW
int result1; // ZAWIERIA LICZBE STRZALOW PLAYERA 1
int result2; // ZAWIERIA LICZBE STRZALOW PLAYERA 2

// INICJALIZIUJE DRUGA GRE NP. CZYSCI ZMIENNE
void Second_game()
{
	lcd_clear();
	lcd_print(1,1,"PRESS SELECT");
	lcd_print(2,1,"TO CONTINUE");
	printf("PRESS SELECT ON LCD TO CONTINUE \n");
	while (Wait_for_select())
	Init_tab();
	lcd_clear();
	printf("Player LCD place starting position of his ship \n");
    lcd_char(1,1,0x01);
	position_x=1;
	position_y=1;
	round_zero=0;
	LCD_player_move();
	tab[position_y-1][position_x-1]=1;
	printf("Ship placed! Now you can use 'shoot y x' to destroy the ship \n\n");
	Print_tab();
    HAL_Delay(500);
    win=0;
}
// DO UART
int System_Interface(const char *command)
{
	if(strstr(command, "shoot") == command)
	{
		uint32_t y, x;
	    if (sscanf(command, "shoot %d %d", &y, &x) == 2) // Sprawdza czy wczytano dwie liczby
	    {
			printf("Fire! on position %d x %d \n", y, x);
			if(y>4 || x>16)
			{
				printf("> Out of the border!!! TRY AGAIN \n");
				return 0;
			}
			else if(tab[y-1][x-1] == 2)
			{
				printf("> You just shooted there !!! TRY AGAIN \n");
				return 0;
			}
    		shoot_counter++;
			if(tab[y-1][x-1] == 1)
			{
				win=1;
				game++;
				int jj=11;
				for(int j = 1; j <= 7 ; j++)
				{
					lcd_clear();
					lcd_print(1,j,"LOSE!");
					lcd_print(2,jj--,"LOSE!");
					printf("Win! \n");
					HAL_Delay(300);
				}
				if (game<3)
				{
					result1 = shoot_counter;
					shoot_counter = 0;
				}
				else
				{
				    char results[16];
				    result2 = shoot_counter;
				    sprintf(results, "P1: %d - P2: %d", result1, result2);
					printf("GAME OVER ! \n");
					printf(results);
					printf("\n");
					lcd_print(1,1,"GAME OVER !");
					lcd_print(2,1,results);
					HAL_Delay(999999999);
				}
			}
			else
			{
				//Informuje osobe strzelajaco gdzie znajduje sie statek
				if(x>position_x)
				{
					printf("\n ----------> Ship is on the left <---------- \n\n");
				}
				else if(x<position_x)
				{
					printf("\n ----------> Ship is on the right <---------- \n\n");
				}
				else
				{
					printf("\n ----------> Ship is on the same column <---------- \n\n");
				}
				Change_lcd(y,x);
				Print_tab();
				return 1;
			}
		}
	    else
	    {
	        printf("> Wrong arguments ! TRY AGAIN \n");
		    return 0;
	    }
	}
	else
	{
		printf("> Wrong command ! TRY AGAIN \n");
	    return 0;
	}
    printf("\n");
    return 0;
}

//MENU DO GRY W USART/LCD
void Menu()
{
	printf("\r\n\r\n");
	printf(" _______  _______  _______  _______  ___   _  ___  \r\n");
	printf("|       ||       ||   _   ||       ||   | | ||   | \r\n");
	printf("|  _____||_     _||  | |  ||_     _||   |_| ||   | \r\n");
	printf("| |_____   |   |  |  |_|  |  |   |  |      _||   | \r\n");
	printf("|_____  |  |   |  |       |  |   |  |     |_ |   | \r\n");
	printf(" _____| |  |   |  |   _   |  |   |  |    _  ||   | \r\n");
	printf("|_______|  |___|  |__| |__|  |___|  |___| |_||___| \r\n\r\n");
	lcd_print(1, 6, "STATKI");
	lcd_char(1,1,0x00);
	lcd_char(1,16,0x00);
	lcd_char(2,4,0x01);
	lcd_char(2,13,0x01);
	lcd_char(2,7,0x00);
	lcd_char(2,10,0x00);
	printf("Press SELECT on LCD to start the game\r\n\r\n");
	while(Wait_for_select());
}
int attempt = 1;
int rounds = 1;
char buffor[32];

// ROZPOCZYNA GRE
void Play_game()
{
	  while(!win)
	  {
		  while(attempt<4)
		  {
			  if(win)
			  {
				  break;
			  }
			  printf("\n -----------> Round - %d <---------- \n\n",rounds);
		  	  printf("Player UART guess where the ship is (3 shots)\n");
		  	  printf("Round %d Attempt %d\n",rounds,attempt);
		  	  printf("> ");
		  	  fflush(stdin);
		  	  fflush(stdout);
		  	  if ( _read(buffor, 32) > 0)
		  	  {
		  		  attempt+=System_Interface(buffor);
		  	  }
		  }
		  if (!win) //
		  {
			  tab[position_y-1][position_x-1]=0;
		  	  printf("Player LCD place new position of his ship \n");
		  	  LCD_player_move();
		  	  tab[position_y-1][position_x-1]=1;
		  	  attempt = 1;
		  	  rounds++;
		  }
	  }
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  // INICJUJE TABLICE NA CALY EKRAN
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ADC1_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  lcd_init(_LCD_4BIT, _LCD_FONT_5x8, _LCD_2LINE);
  lcd_clear();
  //MENU
  Create_char(0,customChar,7);
  Init_tab();
  Menu();
  lcd_clear();

  //RUNDA 0
  printf("Player LCD place starting position of his ship\r\n");
  Create_char(0,customChar,7);
  LCD_player_move();
  printf("Ship placed! Now you can use 'shoot y x' to destroy the ship\r\n");
  tab[position_y-1][position_x-1]=1;
  Print_tab();
  HAL_Delay(500);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  Play_game();
  attempt = 1;
  rounds = 1;
  Second_game();
  Play_game();
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_MultiModeTypeDef multimode = {0};
  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc1.Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the ADC multi-mode
  */
  multimode.Mode = ADC_MODE_INDEPENDENT;
  if (HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_5;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_2CYCLES_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LCD_D6_Pin|LCD_D5_Pin|LCD_D4_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LCD_EN_GPIO_Port, LCD_EN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, LCD_D7_Pin|LCD_RS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : LCD_D6_Pin LCD_D5_Pin LCD_D4_Pin */
  GPIO_InitStruct.Pin = LCD_D6_Pin|LCD_D5_Pin|LCD_D4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : LCD_EN_Pin */
  GPIO_InitStruct.Pin = LCD_EN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LCD_EN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LCD_D7_Pin LCD_RS_Pin */
  GPIO_InitStruct.Pin = LCD_D7_Pin|LCD_RS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
