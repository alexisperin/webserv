/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yhuberla <yhuberla@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/25 13:37:52 by yhuberla          #+#    #+#             */
/*   Updated: 2023/06/07 15:17:08 by yhuberla         ###   ########.fr       */
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
	if (new_string.back() == ' ')
		new_string.pop_back();
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

void run_script(int socket_fd, std::string body)
{
	pid_t pid = fork();
	if (pid == -1)
		throw Webserv::SystemCallException();
	if (!pid)
	{
		char **args = new char *[4];
		args[0] = new char[11];
		strcpy(args[0], "cgi_script"); // I guess we can't strcpy
		args[1] = ft_itoa(socket_fd);
		args[2] = new char[body.size() + 1];
		strcpy(args[2], body.c_str());
		args[3] = NULL;
		execve("cgi_script", args, NULL);
		perror("exec");
		std::cerr << "execve failure args = ";
		for (int index = 0; index < 3; index++)
		{
			std::cerr << args[index] << ", ";
		}
		std::cerr << std::endl;
		exit(1);
	}
	waitpid(pid, NULL, 0);
}

std::string get_body(std::string bufstr)
{
	size_t index = bufstr.find("\r\n\r\n");
	if (index == std::string::npos)
		return ("");
	return (bufstr.substr(index + 4));
}

static int	ft_itoa_len(long nbr)
{
	int	len;

	len = 1;
	if (nbr < 0)
	{
		len++;
		nbr = -nbr;
	}
	while (nbr > 9)
	{
		len++;
		nbr /= 10;
	}
	return (len);
}

static void	ft_itoa_recursive(long nbr, char *str, int index)
{
	if (nbr > 9)
		ft_itoa_recursive(nbr / 10, str, index - 1);
	str[index] = (nbr % 10) + 48;
}

char	*ft_itoa(int n)
{
	long	nbr;
	char	*str;
	int		i;
	int		len;

	nbr = (long) n;
	len = ft_itoa_len(nbr);
	str = new char[len + 1];
	str[len] = 0;
	i = 0;
	if (nbr < 0)
	{
		str[i] = '-';
		nbr = -nbr;
		i++;
	}
	ft_itoa_recursive(nbr, str, len - 1);
	return (str);
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

char **get_execve_args(std::string file_path)
{
	size_t size = file_path.size();
	if (size > 3)
	{
		if (!file_path.compare(size - 3, 3, ".py"))
		{
			char **res = new char *[3];
			res[0] = ft_strdup("python3");
			res[1] = ft_strdup(file_path.c_str());
			res[2] = NULL;
			return (res);
		}
		if (!file_path.compare(size - 3, 3, ".pl"))
		{
			char **res = new char *[3];
			res[0] = ft_strdup("perl");
			res[1] = ft_strdup(file_path.c_str());
			res[2] = NULL;
			return (res);
		}
	}
	char **res = new char *[2];
	res[0] = ft_strdup(file_path.c_str());
	res[1] = NULL;
	return (res);
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