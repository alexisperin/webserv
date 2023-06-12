/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yhuberla <yhuberla@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/25 13:37:52 by yhuberla          #+#    #+#             */
/*   Updated: 2023/06/12 13:55:44 by yhuberla         ###   ########.fr       */
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
		std::getline( indata, line );
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

int check_correct_host(std::string bufstr, std::list<std::string> server_names)
{
	size_t index = bufstr.find("Host: ");
	if (index == std::string::npos)
		return (1);
	size_t multiple_host = bufstr.find("Host: ", index + 1);
	if (multiple_host != std::string::npos)
		return (1);
	
	std::list<std::string>::iterator it = server_names.begin();
	std::list<std::string>::iterator ite = server_names.end();
	for (; it != ite; it++)
	{
		if (!bufstr.compare(index + 6, (*it).size(), *it))
			return (0);
	}
	return (bufstr.compare(index + 6, 9, "localhost") && bufstr.compare(index + 6, 9, "127.0.0.1"));
}

int check_header_names(std::string bufstr)
{
	size_t index = bufstr.find("\r\n");
	while (index != std::string::npos)
	{
		index += 2;
		size_t endl_index = bufstr.find("\r\n", index);
		if (endl_index != std::string::npos && endl_index == index)
			return (0);
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
		char c = str[index];

		switch (c)
		{
			case '\\':
				std::cout << "\\";
				break;
			case '\n':
				std::cout << "\\n\n";
				break;
			case '\r':
				std::cout << "\\r";
				break;
			case '\t':
				std::cout << "\\t";
				break;

			// TODO: Add other C character escapes here.  See:
			// <https://en.wikipedia.org/wiki/Escape_sequences_in_C#Table_of_escape_sequences>

			default:
				if (isprint(c))
				{
					std::cout << c;
				}
				else
				{
					std::cout << '|' << c + '0' << '|';
				}
				break;
		}
	}
	std::cout << "EOF" << std::endl;
}

std::string get_body(std::string bufstr)
{
	size_t index = bufstr.find("\r\n\r\n");
	if (index == std::string::npos)
		return ("");
	return (bufstr.substr(index + 4));
}

std::string GET_content_type(std::string file)
{
	size_t size = file.size();
	if (size > 3)
	{
		if (!file.compare(size - 4, 4, ".css"))
			return ("text/css");
		else if (!file.compare(size - 4, 4, ".png"))
			return ("image/png");
		else if (!file.compare(size - 4, 4, ".ico"))
			return ("image/vnd.microsoft.icon");
		else if (!file.compare(size - 4, 4, ".pdf"))
			return ("application/pdf");
		else if (!file.compare(size - 4, 4, ".gif"))
			return ("image/gif");
		else if (!file.compare(size - 4, 4, ".jpg"))
			return ("image/jpg");
		else if (size > 4)
		{
			if (!file.compare(size - 5, 5, ".html"))
				return ("text/html");
			else if (!file.compare(size - 5, 5, ".jpeg"))
				return ("image/jpeg");
		}
	}
	return ("text/plain");
}

char *ft_strdup(std::string str)
{
	char *res = new char[str.size() + 1];
	size_t index = 0;
	for (; index < str.size(); index++)
		res[index] = str[index];
	res[index] = '\0';
	return (res);
}

void ft_strcat(std::string src, char *dst)
{
	size_t cpyndex = 0;
	for (; dst[cpyndex]; cpyndex++);
	for (size_t index = 0; src[index]; index++, cpyndex++)
		dst[cpyndex] = src[index];
	dst[cpyndex] = '\0';
}

char **map_to_array(std::map<std::string, std::string> env_map)
{
	char **ret = new char *[env_map.size() + 1];

	std::map<std::string, std::string>::iterator it = env_map.begin();
	std::map<std::string, std::string>::iterator ite = env_map.end();

	size_t index = 0;
	for (; it != ite; it++, index++)
	{
		ret[index] = new char[it->first.size() + 1 + it->second.size() + 1];
		ret[index][0] = '\0';
		ft_strcat(it->first, ret[index]);
		ft_strcat("=", ret[index]);
		ft_strcat(it->second, ret[index]);
	}
	ret[index] = NULL;
	return (ret);
}

std::string get_last_word(std::string str)
{
	size_t index = str.rfind('/');
	if (index == std::string::npos)
		return ("/");
	if (index != str.size() - 1)
		return (str.substr(index));
	if (index == 0)
		return (str);
	size_t prev_index = str.rfind('/', index - 1);
	if (prev_index == std::string::npos)
		return (str);
	while (prev_index == index - 1)
	{
		index = prev_index;
		prev_index = str.rfind('/', index - 1);
		if (prev_index == std::string::npos)
			return (str);
	}
	return (str.substr(prev_index));
}
