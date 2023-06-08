/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cgi_quotes.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yhuberla <yhuberla@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/06 14:25:29 by aperin            #+#    #+#             */
/*   Updated: 2023/06/07 14:11:26 by yhuberla         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

int main()
{

	srand(time(0));
	std::ifstream file("quotes.txt");
	std::string quote;
	int	i = (rand() % 100) + 1;

	if (!file.is_open())
	{
		std::cout << "HTTP/1.1 500 Internal Server Error\n\n";
		return 1;
	}
	while(!file.eof() && i)
	{
		std::getline(file, quote);
		i--;
	}
	file.close();

	std::string html_content;
	std::ostringstream content_length;

	html_content += "<html><head><link rel=\"stylesheet\" href=\"../css/quote.css\"></head>";
	html_content += "<body><div><div class=\"wave\"></div><div class=\"wave\"></div>";
	html_content += "<div class=\"wave\"></div></div><div class=\"container\">";
	html_content += "<div class=\"box\"><div class=\"text\">" + quote + "</div>";
	html_content += "<a href=\"quotes.cgi\"><button class=\"button button1\">Next quote</button></a>";
	html_content += "</div></div></body></html>";

	std::string content = "HTTP/1.1 200 OK\n";
	content_length << html_content.size();

	content += "Content-Type:text/html\nContent-Length: ";
	content += content_length.str() + "\n\n" + html_content;
	std::cout << content << std::endl;
	return 0;
}