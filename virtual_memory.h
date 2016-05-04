
#include <vector>
#include <map>

using namespace std;
class Virtual_memory
{
public:
	Virtual_memory()
	{
		first_used = -1;
		last_used = -1;
		left_unused = 3;
		frame[0] = -1;frame[1] = -1;frame[2] = -1;
	}
	int get_first_used(){return first_used;}
	int get_last_used(){return last_used;}
	int get_left_unused(){return left_unused;}
	void set_first_used(int a){first_used = a;}
	void set_last_used(int a){last_used = a;}
	void set_left_unused(int a ){left_unused = a;}
	int frame[3];
private:
	int first_used;//First-in page in the frame
	int last_used;//Least-recently page in the frame
	int left_unused;//left unused frame
	
};