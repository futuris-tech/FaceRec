#include <iostream>
#include <iomanip>
#include <direct.h>
#include <thread>
#include <atomic>
#include <random>
#include <chrono>
#define _USE_MATH_DEFINES
#include <math.h>
#include "gifdec.h"
using namespace std;

const char* const standart_type[] = {
	"normal",
	"noglasses",
};
const char* const test_type[] = {
	"centerlight",
	"glasses",
	"happy",
	"leftlight",
	"rightlight",
	"sad",
	"sleepy",
	"surprised",
	"wink",
};
const int color_group_size = 12;
const int group_num = 255 / color_group_size + 1;
const int people_num = 15;
const int standart_type_num = sizeof(standart_type) / sizeof(char*);
const int test_type_num = sizeof(test_type) / sizeof(char*);

struct image {
	int id;
	char type[12];
};
struct file_name {
	char value[22];
	file_name(image im) {
		strcpy(value, "subject");
		value[9] = '.';
		set(im);
	}
	void set(image im) {
		set_id(im.id);
		set_type(im.type);
	}
	void set_id(int id) {
		id++;
		value[7] = id / 10 + '0';
		value[8] = id % 10 + '0';
	}
	void set_type(const char* type) {
		strcpy(value + 10, type);
	}
};
struct attr_vector {
	uint32_t value[group_num];

	attr_vector() {}
	attr_vector(image im) { set(im); }
	void set(image im) {
		auto f_name = file_name(im);
		auto file = gd_open_gif(f_name.value);
		gd_get_frame(file);

		memset(value, 0, sizeof(value));
		auto count = file->width * file->height;
		for (int i = 0; i < count; i++)
			value[file->frame[i] / color_group_size]++;
		/*default_random_engine dre(chrono::system_clock::now().time_since_epoch().count());
		auto count_16 = count;
		auto cx = (file->width-1) * 0.5;
		auto cy = (file->height-1) * 0.5;
		auto r_max = min(cx, cy);
		for (int i = 0; i < count_16; i++) {
			auto a = dre() * (M_PI * 2.0 / 0x100000000ull);
			auto r = sqrt(dre() * (1.0f / 0xffffffffull)) * r_max;
			auto x = cx + cos(a) * r;
			auto y = cy + sin(a) * r;
			value[file->frame[(int)x + (int)y*file->width] / color_group_size]++;
		}*/

		gd_close_gif(file);
	}

	int find(attr_vector* begin, int size) {
		int minId = -1;
		int minS = INT_MAX;
		for (int i = 0; i < size; i++) {
			auto attr = begin + i;
			int s = 0;
			for (int j = 0; j < group_num; j++) {
				int x = attr->value[j] - value[j];
				s += x * x;
			}
			if (minS > s) {
				minS = s;
				minId = i;
			}
		}
		return minId;
	}
};
struct standart_image : image {
	attr_vector attr;
};
standart_image standart[standart_type_num * people_num];
struct test_image : image {
	standart_image* found;
	void find_standart() {
		attr_vector attr(*this);

		int minS = INT_MAX;
		for (auto& st : standart) {
			int s = 0;
			for (int j = 0; j < group_num; j++) {
				int x = attr.value[j] - st.attr.value[j];
				s += x * x;
			}
			if (minS > s) {
				minS = s;
				found = &st;
			}
		}
	}
};
test_image test[test_type_num * people_num];
const int standart_num = sizeof(standart) / sizeof(standart_image);
const int test_num = sizeof(test) / sizeof(test_image);

int cpu_num;
atomic<int> done_num = 0;
inline void add_done() {
	printf("%d%%\r", ++done_num * 100 / (standart_num + test_num));
}
void init_work(int thread_id) {
	for (int i = thread_id; i < standart_num; i += cpu_num) {
		auto& st = standart[i];
		st.id = i / standart_type_num;
		strcpy(st.type, standart_type[i % standart_type_num]);
		st.attr.set(st);
		add_done();
	}
}
void compare_work(int thread_id) {
	for (int i = thread_id; i < test_num; i += cpu_num) {
		auto& t = test[i];
		t.id = i / test_type_num;
		strcpy(t.type, test_type[i % test_type_num]);
		test[i].find_standart();
		add_done();
	}
}

int main() {
	_chdir("C:\\Users\\admin\\Downloads\\yalefaces");
	cpu_num = thread::hardware_concurrency();
	cout << left << "0%\r";

	const auto threads = new thread[cpu_num];
	for (int i = 0; i < cpu_num; i++)
		threads[i] = thread(init_work, i);
	for (int i = 0; i < cpu_num; i++)
		threads[i].join();

	for (int i = 0; i < cpu_num; i++)
		threads[i] = thread(compare_work, i);
	for (int i = 0; i < cpu_num; i++)
		threads[i].join();
	delete[] threads;

	int score = 0;
	int type_id = 0;
	for (auto& t : test) {
		if (t.id == t.found->id)
			score++;
		if (!type_id) cout << '\n';

		auto f_name = file_name(t);
		cout << setw(22) << f_name.value << "-> ";
		f_name.set(*t.found);
		cout << f_name.value << '\n';

		if (++type_id == test_type_num)
			type_id = 0;
	}

	cout << "\nscore = " << score * 100.0f / test_num << "%\n";
	system("pause");
	return 0;
}