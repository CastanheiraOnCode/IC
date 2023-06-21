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
                //std::cout << std::bitset<8>(buffer) << std::endl; //output buffer
                buffer = 0; //reset buffer

           }

            if (i==(n-1) && remainder!=0)
            {   


                //mask last n bits
                buffer=buffer; 
                buffer = buffer >> (8-remainder);
                buffer = buffer << (8-remainder);

                encode.write(&buffer, 1);
                //std::cout << std::bitset<8>(buffer) << std::endl; //output buffer
                buffer = 0; //reset buffer
            }
            
           
        }
     }

    int getRemainder(vector<string> data){
    long count =0;
    for(uint32_t j=0; j<data.size(); j++){
        count += data[j].size()%8;
    }
    int remainder = count%8;

    return remainder;
   }
    
    
    void encodeSound(vector<string> data){
        ofstream outputFile;    //create file
        outputFile.open("encodedOutput.bin", ios::out | ios::binary);    //output, append, binary
        if (!outputFile.is_open())  cout << "couldnt open file"<<endl;      //debug

        int count=0;
        int remainder = getRemainder(data);
        int fileChars;
        string fileContent;
        //iterate over array
        for(uint32_t j=0; j<data.size(); j++){

            fileContent = data[j];              //current word/entry/string
            fileChars = data[j].size();         //current entry size
            

            for (int i = 0; i < fileChars; i++){
                char temp = (fileContent[i] & 0x01);
                temp=(temp)<<((8-(i%8) -1));
                buffer= buffer | temp;
                
                count++;
                

                if (count%8==0){    
                    //append to file
                    outputFile.write(&buffer, 1);
                    //std::cout << std::bitset<8>(buffer); //output buffer
                    buffer = 0; //reset buffer
                    count=0;
                }

                if (j==(data.size()-1) && i==(fileChars-1) && remainder!=0){
                    //mask last n bits
                    buffer=buffer; 
                    buffer = buffer >> (8-remainder);
                    buffer = buffer << (8-remainder);

                    outputFile.write(&buffer, 1);
                    //std::cout << std::bitset<8>(buffer) << std::endl; //output buffer
                    buffer = 0; //reset buffer
                }
            }
        }

        outputFile.close();
    }
    
    
    void decode(std::ifstream& file, int n){

        //number of chars to read
        std::ofstream decode("decoded.txt", std::ios::binary);
        
        int remainder=n%8;

        for (int i = 0; i < n/8; i++)
        {
           int character = file.get();
            for (int j = 0; j < 8; j++)
            {
                //write to file
                
                int temp = (character >> (7-j)) & 1;
                string toPrint = to_string(temp);
                decode << toPrint;
                //printf("%d",(character >> 7-j) & 1 );
            }

        }

        if(remainder!=0){
            //get nth byte of file

            int character = file.get();
            

            for (int j = 0; j < remainder; j++)
            {
                //write to file
                
                int temp = (character >> (7-j)) & 1;
                string toPrint = to_string(temp);
                decode << toPrint;
                //printf("%d",(character >> 7-j) & 1 );
            }
            
            
        }
    }


     //function to return a small string of decoded bits used to read header of compressed files
     string decodeSmall(std::ifstream& file, int n){

        //number of chars to read
        
        int remainder=n%8;

        //initiate empty string named output
        string output = "";
        

        for (int i = 0; i < n/8; i++)
        {
           int character = file.get();
            for (int j = 0; j < 8; j++)
            {
                //write to file
                
                int temp = (character >> (7-j)) & 1;
                string toPrint = to_string(temp);
                //concatenate output and toPrint
                output = output + toPrint;
                //printf("%d",(character >> 7-j) & 1 );
            }

        }

        if(remainder!=0){
            //get nth byte of file

            int character = file.get();
           

            for (int j = 0; j < remainder; j++)
            {
                //write to file
                
                int temp = (character >> (7-j)) & 1;
                string toPrint = to_string(temp);
                output = output + toPrint;
                //printf("%d",(character >> 7-j) & 1 );
            }
            
            
        }

        return output;
    }
        




    };
