/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cgi_wiki.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yhuberla <yhuberla@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/06 14:25:29 by aperin            #+#    #+#             */
/*   Updated: 2023/06/07 16:48:39 by yhuberla         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <sstream>
#include <string>

int main()
{
// 	std::string html_content;
// 	std::ostringstream content_length;

// 	html_content += "<html>\
// <head>\
//   <link rel=\"stylesheet\" href=\"../css/wiki.css\">\
// </head> \
// <body>\
//   <div class=\"container\">\
//     <img class=\"image\" src=\"../images/wiki.png\" alt=\"Wiki Logo\" width=\"100\" height=\"100\">\
//     <div class=\"box\">\
//       <div class=\"text\">what u want ?</div>\
// 	    <a href=\"https://en.wikipedia.org\"><button class=\"button button1\">Search wiki</button></a>\
//     </div>\
// </div>\
// </body>\
// </html>";  TODO + -> _

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
	// content_length << html_content.size();

	content += "location: https://en.wikipedia.org/wiki/" + search + "\n\n";
	// content += "Content-Type:text/html\nContent-Length: ";
	// content += content_length.str() + "\n\n" + html_content;
	std::cout << content << std::endl;
	return 0;
}