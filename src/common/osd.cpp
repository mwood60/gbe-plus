// GB Enhanced+ Copyright Daniel Baxter 2018
// Licensed under the GPLv2
// See LICENSE.txt for full license text

// File : osd.cpp
// Date : April 14, 2018
// Description : GBE+ On-Screen Display
//
// Loads font for OSD messages. Draws OSD messages to a given SDL surface

#include <iostream>
#include <fstream>

#include "config.h"

/****** Loads a font into a buffer to be drawn by a core for OSD ******/
bool load_osd_font()
{
	//Should be called after parse_ini_file() to get correct data path
	std::string font_file = config::data_path + "fonts/block_font.bin";

	std::ifstream file(font_file.c_str(), std::ios::binary);

	if(!file.is_open())
	{
		std::cout<<"GBE::Could not open font file " << font_file << ". Check file path or permissions. \n";
		config::use_osd = false;
		return false; 
	}

	//Get the file size
	file.seekg(0, file.end);
	u32 file_size = file.tellg();
	file.seekg(0, file.beg);

	u8 font_byte = 0;
	u32 font_size = file_size / 64;
	u32 buffer_pos = 0;
	
	//Resize font buffer
	config::osd_font.resize(font_size * 64);

	//Expand 1BPP font to 32-bit ARGB
	for(u32 x = 0; x < font_size; x++)
	{
		for(u32 height = 0; height < 8; height++)
		{
			for(u32 width = 0; width < 8; width++)
			{
				file.read((char*)&font_byte, 0x1);
								
				buffer_pos = (x * 64) + (height * 8) + width;

				//Draw font to buffer (yellow color)
				config::osd_font[buffer_pos] = (font_byte) ? 0xFFFFE500 : 0xFF000000;
			}
		}
	}

	file.close();

	std::cout<<"GBE::Loaded OSD font\n";

	return true;
} 

/****** Draws an OSD message onto a given buffer ******/
void draw_osd_msg(std::vector <u32> &osd_surface)
{
	//Abort OSD drawing if 1) OSD disabled, 2) message size is zero, 3) given buffer is less than 20 8x8 tiles
	if(!config::use_osd) { return; }
	if(config::osd_message.size() == 0) { return; }
	if(osd_surface.size() < 1280) { return; }

	u32 chr_offset = 0;
	u32 buffer_pos = 0;
	u32 chr_pos = 0;
	u8 current_chr = 0;

	//Limite message size to 20 characters.
	u8 message_size = (config::osd_message.size() <= 20) ? config::osd_message.size() : 20;

	//Cycle through every character
	for(u32 x = 0; x < config::osd_message.size(); x++)
	{
		current_chr = config::osd_message[x];

		//Convert ASCII text to font offsets
		if(current_chr == 0x20) { chr_offset = 0; }
		else if((current_chr >= 0x30) && (current_chr <= 0x39)) { chr_offset = (current_chr - 0x2F); }
		else if((current_chr >= 0x41) && (current_chr <= 0x5A)) { chr_offset = (current_chr - 0x36); }
		else if((current_chr >= 0x61) && (current_chr <= 0x7A)) { chr_offset = (current_chr - 0x56); }
		else { chr_offset = 0; }

		for(u32 osd_h = 0; osd_h < 8; osd_h++)
		{
			for(u32 osd_w = 0; osd_w < 8; osd_w++)
			{
				buffer_pos = (x * 8) + (osd_h * config::sys_width) + osd_w;
				chr_pos = (chr_offset * 64) + (osd_h * 8) + osd_w;

				osd_surface[buffer_pos] = config::osd_font[chr_pos];
			}
		}
	}
}
