/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cgi_wiki.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aperin <aperin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/06 14:25:29 by aperin            #+#    #+#             */
/*   Updated: 2023/06/15 14:24:21 by aperin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <sstream>
#include <string>

int main()
{
	std::string input;
	std::cin >> input;
	std::cerr << "INFO READ IN CGI: " << input << std::endl;

	for (size_t index = 0; input[index]; index++)
	{
		if (input[index] == '+')
			input[index] = '_';
	}

	size_t index = input.find('=');
	std::string search;
	if (index != std::string::npos)
		search = input.substr(index + 1);
	else
		search = "42_(school)";
	
	if (search.empty())
		search = "42_(school)";

	std::string content = "HTTP/1.1 307 Temporary Redirect\n";

	content += "location: https://en.wikipedia.org/wiki/" + search + "\n\n";
	std::cout << content << std::endl;
	return 0;
}