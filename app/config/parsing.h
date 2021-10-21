/**
 * @file parsing.h
 * @brief Definition of the parsing association structure
 * @author  Yncrea HdF - ISEN Lille / Alexis.C, Ali O.
 * @version 0.1
 * @date March 2019, Revised in August 2019
 */

#ifndef CONFIG_PARSING_H_
#define CONFIG_PARSING_H_

typedef struct {
	unsigned int dataOutputType;
	int (*parsingFunctionBinary)(void * data, unsigned char * dest);
	int (*parsingFunctionASCII)(unsigned int instrumentType, void * pData, unsigned char *dest, unsigned int lineReturn);
	int (*parsingFunctionstruct)(unsigned char *source, unsigned int size, void * pData);

} parsing_assoc_t;

#endif /* CONFIG_PARSING_H_ */
