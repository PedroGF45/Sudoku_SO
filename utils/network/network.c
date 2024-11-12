#include <stdio.h>
#include <errno.h>
#include "../logs/logs.h"
#include "network.h"

/* Funções utilitárias retiradas de "UNIX Networking Programming" */


/**
 * Lê exatamente nbytes de um ficheiro/socket.
 *
 * @param fd O descritor de ficheiro (socket ou ficheiro) de onde será feita a leitura.
 * @param ptr Um pointer para o buffer onde os dados lidos serão armazenados.
 * @param nbytes O número de bytes a serem lidos.
 * @return O número de bytes lidos ou um valor negativo em caso de erro.
 */

int readn(int fd, char *ptr, int nbytes)
{
	int nleft, nread;

	nleft = nbytes;
	while (nleft > 0)
	{
		nread = read(fd, ptr, nleft);
		if (nread < 0)
			return (nread);
		else if (nread == 0)
			break;

		nleft -= nread;
		ptr += nread;
	}
	return (nbytes - nleft);
}


/**
 * Escreve exatamente nbytes num ficheiro/socket.
 * 
 * @param fd O descritor de ficheiro (socket ou ficheiro) onde será feita a escrita.
 * @param ptr Um pointer para o buffer que contém os dados a serem escritos.
 * @param nbytes O número de bytes a serem escritos.
 * @return O número de bytes escritos ou um valor negativo em caso de erro.
 */

int writen(int fd, char *ptr, int nbytes)
{
	int nleft, nwritten;

	nleft = nbytes;
	while (nleft > 0)
	{
		nwritten = write(fd, ptr, nleft);
		if (nwritten <= 0)
			return (nwritten);

		nleft -= nwritten;
		ptr += nwritten;
	}
	return (nbytes - nleft);
}


/**
 * Lê uma linha de um ficheiro/socket (até encontrar '\n', atingir maxlen ou ocorrer um erro).
 *
 * @param fd O descritor de ficheiro (socket ou ficheiro) de onde será feita a leitura.
 * @param ptr Um pointer para o buffer onde a linha lida será armazenada.
 * @param maxlen O número máximo de caracteres a serem lidos, incluindo o terminador '\0'.
 * @return O número de caracteres lidos, incluindo '\n', ou -1 em caso de erro. Retorna 0 
 * se atingir o fim do ficheiro antes de ler qualquer carácter.
 */

int readline(int fd, char *ptr, int maxlen)
{
	int n, rc;
	char c;

	for (n = 1; n < maxlen; n++)
	{
		if ((rc = read(fd, &c, 1)) == 1)
		{
			*ptr++ = c;
			if (c == '\n')
				break;
		}
		else if (rc == 0)
		{
			if (n == 1)
				return (0);
			else
				break;
		}
		else
			return (-1);
	}

	/* Não esquecer de terminar a string */
	*ptr = 0;

	/* Note-se que n foi incrementado de modo a contar
	   com o \n ou \0 */
	return (n);
}
