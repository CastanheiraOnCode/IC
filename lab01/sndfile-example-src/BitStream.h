//class to read bits from file

#include <iostream>
#include <vector>
#include <sndfile.hh>
#include <map>
#include <sndfile.hh>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <bitset>
#include <string>
#include <filesystem>

using namespace std;

//this function reads outputs a buffer of 1 byte from chars read from a file
class BitStream
{
public:
    


    //map of 8 bit characters binary codes
    

    char buffer = 0;
   
    
    
    void encode(std::ifstream& file, int n){
        //create encode.bin file
        char* array = new char[n];
        file.read(array, n);
        const int remainder=n%8;

        std::ofstream encode("encode.bin", std::ios::binary);
        int count =0;
        //iterate over array
        for (int i = 0; i < n; i++)
        {
           char temp = (array[i] & 0x01);
           temp=(temp)<<((8-(i%8) -1));
           buffer= buffer | temp;
           
           count++;
           

           if (count%8==0)
           {    

                //append to file
                encode.write(&buffer, 1);
                std::cout << std::bitset<8>(buffer) << std::endl; //output buffer
                buffer = 0; //reset buffer

           }

            if (i==(n-1) && remainder!=0)
            {   


                //mask last n bits
                buffer=buffer; 
                buffer = buffer >> (8-remainder);
                buffer = buffer << (8-remainder);

                encode.write(&buffer, 1);
                std::cout << std::bitset<8>(buffer) << std::endl; //output buffer
                buffer = 0; //reset buffer
            }
            
           
        }
     }
    
    
    void decode(std::ifstream& file, int n){

        //number of chars to read
        std::ofstream decode("decode.txt", std::ios::binary);
        char character;
        int remainder=n%8;

        for (int i = 0; i < n/8; i++)
        {
           int character = file.get();
            for (int j = 0; j < 8; j++)
            {
                //write to file
                
                int temp = (character >> 7-j) & 1;
                string toPrint = to_string(temp);
                decode << toPrint;
                printf("%d",(character >> 7-j) & 1 );
            }

        }

        if(remainder!=0){
            //get nth byte of file

            int character = file.get();
            char* array = new char[remainder];

            for (int j = 0; j < remainder; j++)
            {
                //write to file
                
                int temp = (character >> 7-j) & 1;
                string toPrint = to_string(temp);
                decode << toPrint;
                printf("%d",(character >> 7-j) & 1 );
            }
            
            
        }
    }
        




    };


