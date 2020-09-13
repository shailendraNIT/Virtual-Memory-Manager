#include<bits/stdc++.h>
using namespace std;

vector<pair<int,int> >pageTable;  //creating page table
vector<pair<int,int> >tlb; //creating TLB

int physicalMemory[256][256] = {0};   //RAM ka size h ye 256 frames and page size is 256
int firstAvailableFrame=0;         //phle 0th pos pe dalenge
int tlbHits = 0,pageFaults = 0;
double total = 0;

void matchResult()
{
	FILE *fp1 = fopen("correct.txt","r");
	FILE *fp2 = fopen("output.txt","r");
	
	char str1[200],str2[200];
	int i=0;
	while(fgets(str1,200,fp1) != NULL && fgets(str2,200,fp2)!= NULL)
	{
		i++;
		//cout<<str1<<endl<<str2<<endl;
		if(strcmp(str1,str2) != 0)
		{
			cout<<"Your answers are wrong at line "<<i<<endl;
			return;
		}
	}
	cout<<"Files matched - Your answers are correct!!"<<endl;
	fclose(fp1);
	fclose(fp2);
}

void updateTLB(int pageNo,int frameNo)
{
	int flag=0,i=0; //flag is to check if page is already present in TLB
	for(i=0;i<tlb.size();i++)
	{
		if(tlb[i].first == pageNo)
		{
			flag = 1;  //hme mil gya TLB me vo page number
			break;
		}
	}
	if(flag == 0)
	{
		if(tlb.size()<16) //matlab abhi jagh khali hai TLB me
		{
			tlb.push_back(make_pair(pageNo,frameNo));
		}
		else //jgh khali nhi hai LRU ko replace krdenge 
		{
			tlb.erase(tlb.begin());
			tlb.push_back(make_pair(pageNo,frameNo));
		}
	}
	else //jis pos pe page mila hai vha se erase krke last me dal denge
	{
		tlb.erase(tlb.begin() + i);
		tlb.push_back(make_pair(pageNo,frameNo));
	}
}

void readStore(int pageNo)
{
	FILE *backing_store = fopen("backing_store.bin","rb");
	if(backing_store == NULL)
	{
		cout<<"Couldn't find backing store file"<<endl;
		exit(1);
	}
	if(fseek(backing_store,pageNo*256,SEEK_SET) != 0)
	{
		cout<<"Error getting data from backing_store"<<endl;
		exit(1);
	}
	else
	{
		signed char buffer[256];
		if(fread(buffer,sizeof(signed char),256,backing_store) == 0)
		{
			cout<<"Error reading data from backing_store"<<endl;
			exit(1);
		}
		
		for(int i=0;i<256;i++)  //RAM me daal rhe h hr value ko 
		{
			physicalMemory[firstAvailableFrame][i] = buffer[i];
		}
		
		if(pageTable.size() <256)  //agr PT me jgh h to dal do yha bhi
			pageTable.push_back(make_pair(pageNo,firstAvailableFrame));
		firstAvailableFrame++;
	}
	fclose(backing_store);
}

void getFrameNumber(int logical_address)
{
	int pageNo = ((logical_address & 0xFFFF)>>8); //to get the last 16 bits & shifting right 8 bits gives us page no.
	int offset = (logical_address & 0xFF);     //to get the last 8 bits which are offset
	int frameNo = -1;  //initially we don't have frame no.
	
	for(int i=0;i<tlb.size();i++)  //first we will check in TLB
	{
		if(tlb[i].first == pageNo)
		{
			tlbHits++;
			frameNo = tlb[i].second;
			break;
		}
	}
	if(frameNo == -1)  //if page is not in TLB we will search it in PAGE TABLE
	{
		for(int i=0;i<pageTable.size();i++)
		{
			if(pageTable[i].first == pageNo)
			{
				frameNo = pageTable[i].second;
				break;
			}
		}
	}
	if(frameNo == -1) //if still we dont find page in any of TLB and PT it's a page fault
	{
		pageFaults++;
		readStore(pageNo);   //servicing page fault searching in disk
		frameNo = firstAvailableFrame - 1;  
	}
	
	updateTLB(pageNo,frameNo);   //putting it in TLB
	//cout<<frameNo<<" "<<offset<<endl;
	int val = (frameNo<<8|offset);
	cout<<"Virtual Address : "<<logical_address<<" Physical Address : "<< (frameNo<<8|offset)<<" Value : "<<physicalMemory[frameNo][offset]<<endl;
	ofstream fout;
	fout.open("output.txt",std::ios_base::app); //app=append original file ko overwrite nhi krega usi me append krega
	fout<<"Virtual address: "<<logical_address<<" Physical address: "<< (frameNo<<8|offset)<<" Value: "<<physicalMemory[frameNo][offset]<<endl;
	fout.close();
}

int main()
{
	FILE *address_file = fopen("address.txt","r");
	if(address_file == NULL)
	{
		cout<<"address.txt file not found"<<endl;
		return 0;
	}
	int logical_address;
	char str[100];
	ofstream fout;
	fout.open("output.txt", std::ofstream::out | std::ofstream::trunc);
	fout.close();
	while(fgets(str,10,address_file) != NULL)
	{
		logical_address = atoi(str);
		getFrameNumber(logical_address);
		total++;
	}
	cout<<"Number of TLB Hits : "<<tlbHits<<endl;
	cout<<"Number of Page Faults : "<<pageFaults<<endl;
	cout<<"TLB Hit Rate : "<<tlbHits/total<<endl;
	cout<<"Page Fault Rate : "<<pageFaults/total<<endl;
	matchResult();
		
	fclose(address_file);
	
}
