//-----------Khai báo thư viện chuyên dụng------------------------
#include <iostream> // thư viện iostream, cin, cout
#include <fstream> // Thư viện chuyên để đọc, mở file
#include <sstream>  // Thư viện để đọc số từ xâu
#include <string>   // thư viện xử lí xâu 
#include <unordered_map> // Map chứa 1 cặp key value lần lượt là thanh ghi và giá trị của nó
#include <bitset> // Thư viện đổi cơ số 2
//-----------Khai báo namespace-----------------------------
using namespace std;
//----------Khai báo địa chỉ hiện tại----------------------
int currentaddress=0x00400000;
//----------Khai báo các thanh ghi-------------------------
string Array[32] = {
    "$zero", "$at", "$v0", "$v1", "$a0", "$a1", "$a2", "$a3",
    "$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7",
    "$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7",
    "$t8", "$t9", "$k0", "$k1", "$gp", "$sp", "$fp", "$ra"
    };

// Xóa các chú thích và hàng trống khỏi chương trình
string DeleteComment(const string& line) {
    string result = line;                               // Nhận vào 1 dòng
    size_t pos = result.find('#');                      // Tìm vị trí của #, size_t là kiểu dữ liệu không âm lưu trữ kích cỡ, trả npos nếu không tìm thấy
    if (pos != string::npos) {                          // Check thử pos có phải là giá trị hợp lệ hay không "npos" = "khong hop le"
        result = result.substr(0, pos);                 // nếu có cắt từ 0 -> tới vị trí #, không lấy #
    }

      // Xoá các khoảng trắng ở cuối chuỗi
      pos = result.find_last_not_of(" \t");             // Tìm kí tự cuối cùng không phải là space
    if (pos != string::npos) {
        result = result.substr(0, pos + 1);             // Cắt từ đầu tới nó. Do lệnh substr cắt từ 0 -> a-1 nên cộng 1 vào
    } else return "";

        // Xoá các khoảng trắng thừa ở giữa chuỗi
    bool prev_space = false;        // Kiểm tra xem đã có 1 dấu cách trước đó chưa
    for (size_t i = 0; i < result.size(); i++) {
        if (result[i] == ' ' || result[i] == '\t') {    
            if (prev_space) {
                result.erase(i, 1); // Nếu có rồi thì xoá
                i--;                // Giảm i xuống để không sót
            } else {
                prev_space = true;  // Nếu chưa có thì đánh true
            }
        } else {
            prev_space = false; // Nếu không thì đánh ngược false
        }
    }
    // Xoá các khoảng trắng ở đầu chuỗi
    pos = result.find_first_not_of(" \t");  // Tìm vị trí đầu tiên không phải space
    if (pos != string::npos) {
        result = result.substr(pos);       //Cắt từ đó đến hết 
    }

    return result;                                      // Trả result
}

// Tính toán địa chỉ cho label
unordered_map<string, int> CalculateImmediate(const string& filename) { // Đầu vào là 1 tên file, trả về 1 map bao gồm file ban đầu và địa chỉ
    unordered_map<string, int> labelTable;  // Tạo bảng label
    ifstream file(filename);                // mở file               
    string line;
    int address = 0x00400000;                       // Địa chỉ bắt đầu
    cout<<"Địa chỉ bắt đầu"<<endl;
    cout<<hex<<address<<endl;
    while (getline(file, line)) {
        line = DeleteComment(line);       // Gọi hàm chuẩn hoá, tức là function delete comment
        if (!line.empty()) {              // Nếu trả về là xâu có nghĩa
            if (line.back() == ':') {    // Nếu đuôi của nó là ":" , đã được chuẩn hoá 
                string label = line.substr(0, line.size() - 1); // Cắt cái lable ra, không lấy dấu :
                labelTable[label] = address;                    // Cho cái lable đó địa chỉ
            }
            address += 4;                                       // Địa chỉ cộng 4 sau mỗi dòng lệnh 
        }
    }
    file.close();                        // Đóng file
    return labelTable;                   // Trả địa chỉ
}

// Xây dựng bảng label và xóa label khỏi chương trình
string BuildLabelTable(const string& filename, unordered_map<string, int>& labelTable) { // Nhận tên file, bảng lable
    ifstream file(filename);    // mở file
    stringstream output;        // ghi file
    string line;                // xử lí các dòng
    while (getline(file, line)) {   // lấy các dòng trong file ra
        line = DeleteComment(line); // Chuẩn hoá
        if (!line.empty() && line.back() != ':') {  // Nếu nó khác rỗng và khác :
            output << line << endl; // thì mới in ra  , tại vì phía trên đã cắt bỏ chỉ để lại : thôi 
        }
    }
    file.close();   // đóng file 
    return output.str();    //trả về chuỗi đã được xử lí, ở đây là tất cả nội dung file
}

int direct(string& total){
    for(int i=0;i<32;i++){
        if(total==Array[i]){ // coi thử thanh ghi đó có giá trị là bao nhiêu
            return i;
        }
    }
    return -1;          // không có trả về -1
}



// Tạo mã nhị phân cho lệnh
string GenerateBinary(const string& instruction, const unordered_map<string, int>& labelTable) {
    stringstream output;
    string opcode,funct;
    string rs,rt,rd;
    // Mô tả cách hoạt động
    // Example addi $s0, $t1, 10 
    // B1 :Tiến hành cắt lệnh nhắc ở đầu, cắt tới dấu cách 
    // B2 : So khớp lệnh, chia thành 6 chức năng chính, vì mỗi chức năng sẽ có số từ khác nhau
    // B3 : So khớp thanh ghi, ghi opcode , ghi trường funct hoặc tính address hoặc tính đổi về nhị phân
    // B4 : Ghép mã trả mã máy
    
    size_t pos = instruction.find_first_of(' ');
    string total_process=instruction.substr(0, pos);
    while(instruction[pos]==' ' || instruction[pos]==',' ){
    pos++;
	}
    int type;
    //Tạo opcode, xác định chức năng từng loại lệnh  
    // type{0:R type, 1:I type, 2:J type, 3:branchType, 4:loadType, 5:LiType, 6:memoryacessType}
    if (total_process == "add") {
    opcode = "000000";
    type = 0;
    funct = "100000";} 
    else if (total_process == "addu") {
    opcode = "000000";
    type = 0;
    funct = "100001";} 
    else if (total_process == "and") {
    opcode = "000000";
    type = 0;
    funct = "100100";} 
    else if (total_process == "jr") {
    opcode = "000000";
    type = 0;
    funct = "001000";}
    else if (total_process == "addi") {
        opcode = "001000";
        type=1;
    } 
    else if (total_process == "addiu") {
        opcode = "001001";
        type=1;
    } 
    else if (total_process == "andi") {
        opcode = "001100";
        type=1;
    } 
    else if (total_process == "beq") {
        opcode = "000100";
        type=3;
    } 
    else if (total_process == "bne") {
        opcode = "000101";
        type=3;
    } 
    else if (total_process == "lbu") {
        opcode = "100100";
        type=4;
    } 
    else if (total_process == "lhu") {
        opcode = "100101";
        type=4;
    } 
    else if (total_process == "lui") {
        opcode = "001111";
        type=5;
    }
    else if (total_process == "lw") {
        opcode = "100011";
        type=6;
    } 
    else if (total_process == "sw") {
        opcode = "101011";
        type=6;
    } 
    else if (total_process == "j") {
        opcode = "000010";
        type=2;
    } 
    else if (total_process == "jal") {
        opcode = "000011";
        type=2;
    }

    if (type == 0) {
        int temp_pos=pos;
        rs = instruction.substr(pos);
        pos = rs.find(' ');
        rs = rs.substr(0, pos);
        pos+=temp_pos;
        while(instruction[pos]==' ' || instruction[pos]==',' ){
            pos++;
        }
        bitset<5> rs_bits(direct(rs));  
        rs = rs_bits.to_string();

        temp_pos=pos;
        rt = instruction.substr(pos);
        pos = rt.find(' ');
        rt = rt.substr(0, pos);
        pos+=temp_pos;
        while(instruction[pos]==' ' || instruction[pos]==',' ){
            pos++;
        }
        bitset<5> rt_bits(direct(rt));
        rt = rt_bits.to_string();

        rd = instruction.substr(pos);
        pos++;
        bitset<5> rd_bits(direct(rd));
        rd = rd_bits.to_string();
        output<<opcode<<' '<<rs<<' '<<rt<<' '<<rd<<" 00000 "<<funct ;
        return output.str();
    }
    else if (type == 1) { // I-type
    string rs, rt, immediate;
    int temp_pos=pos;
    rs = instruction.substr(pos);
    pos = rs.find(' ');
    rs = rs.substr(0, pos);
    pos+=temp_pos;   
    while(instruction[pos]==' ' || instruction[pos]==',' ){
        pos++;
    }
    
    bitset<5> rs_bits(direct(rs));
    rs = rs_bits.to_string();

    rt = instruction.substr(pos); // cout<<rt<<endl; 
    temp_pos=pos;  
    pos = rt.find(' ');
    rt = rt.substr(0, pos);
    pos+=temp_pos;
    while(instruction[pos]==' ' || instruction[pos]==',' ){
        pos++;
    }

    bitset<5> rt_bits(direct(rt));
    rt = rt_bits.to_string();


    immediate = instruction.substr(pos);

    int immediate_value = stoi(immediate);
    bitset<16> immediate_bits(immediate_value);
    immediate = immediate_bits.to_string();

    output << opcode << ' ' << rs << ' ' << rt << ' ' << immediate;
    return output.str();}
    else if (type == 2) { // J-type
    string address;
    address = instruction.substr(pos); // nếu gặp nhãn lable
    string address_string;
    int address_value;
    if (labelTable.count(address) > 0) {
        address_value = labelTable.at(address)     & 0x03FFFFFF; // Vứt đi 4 bit đầu
        address_value = address_value << 2; // dịch trái 2 bit
        bitset<26> address_bits(address_value); 
        address_string= address_bits.to_string();
    } else {
        address_string="not found"; // Hàm notfuond sẽ trả về những lable bị lỗi, có nghĩa là tìm không thấy cái lable nó nhảy tới
    }
    output << opcode << ' ' << address_string;
    return output.str();}

    else if (type == 3) { // branchType
    string rs, rt, branchLabel;
    int temp_pos=pos;
    rs = instruction.substr(pos);
    pos = rs.find(' ');
    rs = rs.substr(0, pos);
    pos+=temp_pos;
    while(instruction[pos]==' ' || instruction[pos]==','  ){
    pos++;
	}
	temp_pos=pos;
    rt = instruction.substr(pos);
    pos = rt.find(' ');
    rt = rt.substr(0, pos);
	//cout<<rt<<endl;
    pos+=temp_pos;
    while(instruction[pos]==' ' || instruction[pos]==',' ){
    pos++;
	}
    branchLabel = instruction.substr(pos);
    // branchLabel = branchLabel.substr(0, pos);
    // pos++;
	cout<< "label : "<<branchLabel<<endl; 
    bitset<5> rs_bits(direct(rs));
    string rs_string = rs_bits.to_string();

    bitset<5> rt_bits(direct(rt));
    string rt_string = rt_bits.to_string();

    int branch_offset;
    string offset_string;
    if (labelTable.count(branchLabel) > 0) {
        int branch_address = labelTable.at(branchLabel);
        branch_offset = (branch_address - (currentaddress + 4))/4; // Caculate offset nhảy đến, chia 4 do có 4 world
        bitset<16> offset_bits(branch_offset);
        offset_string = offset_bits.to_string();
         // Calculate the relative offset
    } else {
        offset_string="not found";
    }

    // Convert the branch offset to a binary representation
        output << opcode << ' ' << rs_string << ' ' << rt_string << ' ' << offset_string;

    return output.str();}
     else if (type == 6) { // memoryacessType
     string base, rt, offset;
   
	int temp_pos=pos;
     rt = instruction.substr(pos);
     pos = rt.find_first_of(", ");;
     rt = rt.substr(0, pos);
     pos+=temp_pos;
    while(instruction[pos]==' ' || instruction[pos]==','  ){
    pos++;
	}
	temp_pos=pos;
  	offset = instruction.substr(pos);
    pos = offset.find(" (");
    offset = offset.substr(0, pos);
    pos+=temp_pos;
    while(instruction[pos]==' ' || instruction[pos]==','|| instruction[pos]=='(' ){
    pos++;
	}
	
     base = instruction.substr(pos);
     pos = base.find(')');
     base = base.substr(0, pos);

     bitset<5> base_bits(direct(base));
     string base_string = base_bits.to_string();

     bitset<5> rt_bits(direct(rt));
     string rt_string = rt_bits.to_string();
	 //cout<<rt<<endl<<base<<endl<<offset<<endl;
        // Chú ý: offset string chính là trường imediate
    // // Convert the offset to a 16-bit binary representation

    int immediate_value = stoi(offset, nullptr, 16);
     bitset<16> offset_bits(immediate_value);
     string offset_string = offset_bits.to_string();

     output << opcode << ' '<< rt_string << ' ' << base_string << ' '  << offset_string;

     return output.str();}




    return output.str();
}

// Chuyến 1 - Duyệt tập tin hợp ngữ đầu vào
string FirstPass(const string& filename, unordered_map<string, int>& labelTable) {
    string sourceCode = BuildLabelTable(filename, labelTable);
    labelTable = CalculateImmediate(filename);
    for(auto x : labelTable){
    	cout<<x.first<<' '<<x.second<<endl; // cout ra địa chỉ lable
	}
    return sourceCode;
}

// Chuyến 2 - Duyệt tập tin tạm và tạo mã máy
string SecondPass(const string& sourceCode, const unordered_map<string, int>& labelTable) {
    stringstream output;
    string instruction;
    stringstream ss(sourceCode);
    while (getline(ss, instruction)) {
    	currentaddress+=4;
        cout<<instruction<<endl;
        string binary = GenerateBinary(instruction, labelTable);
        output << binary << endl;
    }
    return output.str();
}


int main() {
    string inputFilename = "/users/cnmeow/documents/it012/mips.txt"; // File mã mips
    string tempFilename = "/users/cnmeow/documents/it012/tempfile.txt"; // File tạm sau khi xoá comment khỏi file mips
    string outputFilename = "/users/cnmeow/documents/it012/machine_code.txt"; // File mã máy sau khi convert
    
    unordered_map<string, int> labelTable;
    string sourceCode = FirstPass(inputFilename, labelTable);
    ofstream tempFile(tempFilename);
    tempFile << sourceCode;
    tempFile.close();

    string machineCode = SecondPass(sourceCode, labelTable);
    ofstream outputFile(outputFilename);
    outputFile << machineCode;
    outputFile.close();

    cout << "Compilation completed. Machine code saved to " << outputFilename << endl;
    return 0;
}
