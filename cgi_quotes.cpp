/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cgi_quotes.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aperin <aperin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/06 14:25:29 by aperin            #+#    #+#             */
/*   Updated: 2023/06/06 14:38:23 by aperin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <fstream>
#include <string>

int main()
{
	srand(time(0));
	std::ifstream file("quotes.txt");
	std::string line;
	int	i = (rand() % 100) + 1;

	if (!file.is_open())
		return 1; // HANDLE ERROR
	while(!file.eof() && i)
	{
		std::getline(file, line);
		i--;
	}
	std::cout << line << std::endl;
	file.close();
}