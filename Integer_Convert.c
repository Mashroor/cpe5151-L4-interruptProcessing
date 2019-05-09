#include "Integer_Convert.h"
#include "AT91SAM7SE512.h"
#include "AT91SAM7SE-EK.h"
#include <math.h>

/***********************************************************************************************************
* Integer_ASCII(int input, unsigned int * outstring)
* Input: A signed 32-bit value, a pointer to an array
* Output: The signed value is converted to a string of ASCII characters (as 32-bit values)
* Cautions: A 0 input will give an output of 0x30 and a null.  A negative value will have a '-' at the start 
*           of the output string.
************************************************************************************************************
* Usage: Declare an array of sufficient length to hold the output string in the function that calls this   
*        function.  Use the name of that array as the pointer parameter.  The output string can be found
*        in that array after the function executes.  The array will have a null (0x00) termination.
***********************************************************************************************************/
void Integer_ASCII(int input, unsigned int * outstring)
{
	int temp, result, remainder;
	unsigned int index, exp;
	index=0;
	if((input&0x80000000)==0x80000000)   // Check for negative value
	{
		input=(~input)+1;                  // and two's complement if negative
		outstring[index]='-';							// and place negative sign in first character
		index++;
  }		
	if(input<10)
	{
		outstring[index]=input|0x30;
	  index++;
	  outstring[index]=0;
	}
	else
	{
		exp=9;
		do
		{
			temp=pow(10,exp);
			result=input/temp;
			exp--;
		}while(result==0);
		outstring[index]=(result)|0x30;
		index++;
		remainder=input%temp;
		while(exp>0)
		{
			temp=pow(10,exp);
			exp--;
			result=remainder/temp;
			outstring[index]=(result)|0x30;
			index++;
			remainder=remainder%temp;
		}
		outstring[index]=(remainder)|0x30;
		index++;
		outstring[index]=0;
	}
}
/***********************************************************************************************************
* Integer_ASCII_unsigned(unsigned int input, unsigned int * outstring)
* Input: An unsigned 32-bit value, a pointer to an array
* Output: The unsigned value is converted to a string of ASCII characters (as 32-bit values)
* Cautions: A 0 input will give an output of 0x30 and a null.  A negative value will seen as a large 
*           unsigned value.
************************************************************************************************************
* Usage: Declare an array of sufficient length to hold the output string in the function that calls this   
*        function.  Use the name of that array as the pointer parameter.  The output string can be found
*        in that array after the function executes.  The array will have a null (0x00) termination.
***********************************************************************************************************/

void Integer_ASCII_unsigned(unsigned int input, unsigned int * outstring)
{
	int temp, result, remainder;
	unsigned int index, exp;
	index=0;
	if(input<10)
	{
		outstring[index]=input|0x30;
	  index++;
	  outstring[index]=0;
	}
	else
	{
		exp=9;
		do
		{
			temp=pow(10,exp);
			result=input/temp;
			exp--;
		}while(result==0);
		outstring[index]=(result)|0x30;
		index++;
		remainder=input%temp;
		while(exp>0)
		{
			temp=pow(10,exp);
			exp--;
			result=remainder/temp;
			outstring[index]=(result)|0x30;
			index++;
			remainder=remainder%temp;
		}
		outstring[index]=(remainder)|0x30;
		index++;
		outstring[index]=0;
	}
}

/***********************************************************************************************************
* Integer_ASCII_Fixed_Width(unsigned int input, unsigned int width, unsigned int * outstring)
* Input: An unsigned 32-bit value, a specifier for number of characters in the output, a pointer to an array
* Output: The unsigned value is converted to a string of ASCII characters (as 32-bit values).
          
* Cautions: The output is left padded with zeroes when the number does not fill the width.
*           If the value is larger than the width allows, a 'v' will be the first character (overflow).
************************************************************************************************************
* Usage: Declare an array of sufficient length to hold the output string in the function that calls this   
*        function.  Use the name of that array as the pointer parameter.  The output string can be found
*        in that array after the function executes.  The array will have a null (0x00) termination.
***********************************************************************************************************/

void Integer_ASCII_Fixed_Width(unsigned int input, unsigned int width, unsigned int * outstring)
{
	int temp, result;
	unsigned int index, exp, j;
	exp=width-1;
	index=0;
	for(j=0;j<(width-1);j++)
	{
		temp=pow(10,exp);
		result=input/temp;
		if(result>9) result=0x46;       // a value too large to fit in width results in 'v' as first character
		exp--;
		outstring[index]=(result)|0x30;
	  index++;
	  input=input%temp;
	}
	outstring[index]=(input)|0x30;
	index++;	
	outstring[index]=0;
}		


