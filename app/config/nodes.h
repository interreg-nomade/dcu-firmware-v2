/**
 * @file nodes.h
 * @brief Contains definition of a "node"
 * @author Alexis.C, Ali O.
 * @version 0.1
 * @date March 2019, Revised in August 2019

 */

#ifndef CONFIG_NODES_H_
#define CONFIG_NODES_H_

typedef struct node_descriptor {
	unsigned int type;
	char name[32];
	int mode;
	int (*decodingFunction)(void* params);
	int (*parsingFunctionBinary)(void * data, unsigned char * dest);
	int (*parsingFunctionASCII)(void* params);
    unsigned int refreshRate;
    unsigned int dataSize;

} node_decriptor;


#endif /* CONFIG_NODES_H_ */
