#pragma once

class LinkedInt {
	LinkedInt* next;
public:
	bool empty;
	LinkedInt() { empty = true; };
	LinkedInt(int* value) { data = value; empty = false; };
	void Add(int* value)
	{
		if (empty == true)
		{
			data = value;
			empty = false;
		}
		else 
		{
			if (next == nullptr)
				next = new LinkedInt(value);
			else
				next->Add(value);
		}
	};
	std::wstring ToString() 
	{
		if (next != nullptr)
			return std::to_wstring(*data) + L" " + next->ToString();
		else 
			return std::to_wstring(*data);
	}
private:
	int* data;
};