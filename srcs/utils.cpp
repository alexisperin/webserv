/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aperin <aperin@student.s19.be>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/25 13:37:52 by yhuberla          #+#    #+#             */
/*   Updated: 2023/05/29 11:52:19 by aperin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "utils.hpp"

std::string	trim_spaces(std::string str)
{
	int index = 0;
	std::string new_string;
	while (str[index] == ' ' || str[index] == '\t')
		++index;
	while (str[index])
	{
		if (str[index] == '\t')
			new_string += ' ';
		else
			new_string += str[index];
		if (str[index] == ' ' || str[index] == '\t')
		{
			while (str[index] == ' ' || str[index] == '\t')
				++index;
			--index;
		}
		++index;
	}
	if (new_string[new_string.size() - 1] == ' ')
		new_string = new_string.substr(0, new_string.size() - 1);
	return (new_string);
}

std::string read_data(std::ifstream &indata)
{
	std::string res;
	std::string line;
	while (!indata.eof()) {
		std::getline(indata, line);
		res += line;
		if (!indata.eof())
			res += '\n';
	}
	indata.close();
	return (res);
}

int is_error_code(int code)
{
	return ((code >= 400 && code <= 418) || (code >= 421 && code <= 426) || (code >= 428 && code <= 431) || code == 451
		|| (code >= 500 && code <= 508) || (code >= 510 && code <= 511));
}

int check_http_version(std::string bufstr)
{
	size_t index = bufstr.find("\r\n");
	if (index == std::string::npos || index < 8)
		return (0);
	return (bufstr.compare(index - 8, 8, "HTTP/1.1"));
}

int check_correct_host(std::string bufstr)
{
	size_t index = bufstr.find("Host: ");
	if (index == std::string::npos)
		return (1);
	size_t multiple_host = bufstr.find("Host: ", index + 1);
	if (multiple_host != std::string::npos)
		return (1);
	//TODO check if host in server_name or not (may put this func as private in class Server)
	return (bufstr.compare(index + 6, 9, "localhost") && bufstr.compare(index + 6, 9, "127.0.0.1"));
}

int check_header_names(std::string bufstr)
{
	size_t index = bufstr.find("\r\n");
	while (index != std::string::npos)
	{
		index += 2;
		size_t endl_index = bufstr.find("\r\n", index);
		size_t end_line = endl_index;
		if (endl_index == std::string::npos)
			end_line = bufstr.size();
		bool has_content = false;
		for (size_t curr_index = index; curr_index < end_line; ++curr_index)
		{
			if (bufstr[curr_index] == ':' && !has_content)
				return (1);
			has_content = true;
			if (bufstr[curr_index] == ':')
				break ;
			if (bufstr[curr_index] == ' ')
				return (1);
		}
		index = endl_index;
	}
	return (0);
}

void display_special_characters(std::string str)
{
	for (size_t index = 0; index < str.size(); ++index)
	{
		int c = str[index];

		switch (c)
		{
			case '\\':
				printf("\\\\");
				break;
			case '\n':
				printf("\\n\n");
				break;
			case '\r':
				printf("\\r");
				break;
			case '\t':
				printf("\\t");
				break;

			// TODO: Add other C character escapes here.  See:
			// <https://en.wikipedia.org/wiki/Escape_sequences_in_C#Table_of_escape_sequences>

			default:
				if (isprint(c))
				{
					putchar(c);
				}
				else
				{
					printf("\\x%X", c);
				}
				break;
		}
	}
}