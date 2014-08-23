#ifndef __CLOTH_LOOP_H__
#define __CLOTH_LOOP_H__

#include <vector>
using namespace std;


class ClothLine;

class ClothLoop
{
public:
	vector<ClothLine*> lines;
	ClothLoop(void);
	~ClothLoop(void);
};

#endif