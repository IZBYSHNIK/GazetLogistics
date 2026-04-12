// Copyright (C) 2026 Ivan Degtyarev

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include <QApplication>
#include <QMainWindow>
#include <QTextStream>
#include <QFileDialog>
#include <QTranslator>
#include "form.h"

#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <vector>
#include <tuple>
#include <filesystem>
#include <tinyxml2.h>
#include <toml++/toml.hpp>

using namespace tinyxml2;

class Newspaper {
    public:
    enum topics {
        Politics = 0,
        EconomyAndFinance,
        Society,
        IncidentsAndCrime,
        CultureAndArt,
        Sports,
        ScienceAndTechnology,
        HealthAndMedicine,
        LifestyleAndEntertainment,
        WeatherAndEcology,
        OpinionsAndAnalytics,
        LettersAndFeedback,
        AdsAndAnnouncements,
        RegionalAndLocalNews,
        InternationalNews,
    };
    std::string title;
    int topic{Politics};
    int index{0};
    std::string editor;
    
    Newspaper() {};
    
    Newspaper(const std::string& title, topics topic, int index, const std::string& editor) : title{title}, topic{topic}, index{index}, editor{editor} {};
};

class PrintingHouse {
    public:
    int id{0};
    std::string title;
    std::string address;
    std::string boss;
    // index from Newspapers and circulation and price for one
    std::map<int, std::tuple<int, float>> newspapers;
    
    PrintingHouse() {};
    
    PrintingHouse(const std::string& title, const std::string& address, const std::string& boss, int id=0) : title{title}, address{address}, boss{boss}, id{id} {};

    bool add_newspaper(int newspaper_index, std::tuple<int, float> circulation_and_price) {
        newspapers[newspaper_index] = circulation_and_price;
        return 0;
    }
    bool del_newspaper(int newspaper_index) {
        newspapers.erase(newspaper_index);
        return 0;
    }
};

class PostOffice {
    public:
    int number{0};
    std::string address;
    std::string region;

    //   ID Printing House | Index Newspaper | Count | Price
    std::map<std::tuple<int, int>, std::tuple<int, float>> printing_houses;

    PostOffice() {};
    PostOffice(int number, const std::string& address, const std::string& region) : number{number}, address{address}, region{region} {};
    bool add_order(std::tuple<int, int> IDPrintingHouse_IndexNewspaper, std::tuple<int, float> count_and_price) {
        printing_houses[IDPrintingHouse_IndexNewspaper] = count_and_price;
        return 0;
    }
    
    bool del_newspaper(std::tuple<int, int> index) {
        printing_houses.erase(index);
        return 0;
    }
};

class BD {
    public:
    std::filesystem::path base{"./data"};
    std::filesystem::path output{"./output"};
    std::string cur_user{""};
    std::filesystem::path language_path{"./resurses/languages/"};
    std::filesystem::path nw{"newspapers.xml"};
    std::filesystem::path ph{"printing_house.xml"};
    std::filesystem::path po{"post_office.xml"};

    std::vector<Newspaper> newspapers;
    std::vector<PrintingHouse> printing_houses;
    std::vector<PostOffice> post_offices;
    
    
    bool load() {
        std::filesystem::path dir_path{base};
        if(!std::filesystem::exists(dir_path)) {
            std::filesystem::create_directory(base);
        }
        return 0;
    }
    
    bool is_free_login(const std::string& login) {
        for (const auto& entry : std::filesystem::directory_iterator(base)) {
            auto filename = entry.path().filename();
            if (entry.is_directory()) {
                if(filename == login)
                return false;
            }
        }
        return true;
    }

    bool is_valid_filename(const std::string& login) {
    if (login.length() == 0 || login.length() > 255) {
        return false;
    }
    const std::string invalidChars = "\\/:*?\"<>|!";
    for (char c : invalidChars) {
        if (login.find(c)!= std::string::npos) {
            return false;
        }
    }
    return true;
}

    bool user_create(const std::string& login) {
        std::filesystem::path lg{login};
        std::filesystem::create_directory(base/lg);
        return 0;
    }

    bool user_del(const std::string& login) {
        std::filesystem::path lg{login};
        std::filesystem::remove_all(base/lg);
        return 0;
    }

    std::vector<std::string> get_users() {
        std::vector<std::string> res;
        for (const auto& entry : std::filesystem::directory_iterator(base)) {
            auto filename = entry.path().filename();
            if (entry.is_directory()) {
                res.push_back(filename);
            }
        }
        return res;
    }

    bool add_newspapers(const Newspaper& t) {
        newspapers.push_back(t);
        return 0;
    }

    bool add_printing_house(const PrintingHouse& t) {
        printing_houses.push_back(t);
        return 0;
    }

    bool add_post_office(const PostOffice& t) {
        post_offices.push_back(t);
        return 0;
    }

    bool del_post_office(int indx) {
        auto it = post_offices.begin() + indx; 
        post_offices.erase(it);
        return 0;
    }

    bool del_newspapers(int indx) {
        // indx for vector, index not from Newspaper
        auto it = newspapers.begin() + indx; 
        newspapers.erase(it);
        return 0;
    }

    bool del_printing_house(int indx) {
        // indx for vector, index not from Newspaper
        auto it = printing_houses.begin() + indx; 
        printing_houses.erase(it);
        return 0;
    }

    void save_user_data(const std::string& user) {
        std::filesystem::path lg{user};
        
        {
            XMLDocument doc;
            auto* declaration = doc.NewDeclaration();
            doc.InsertFirstChild(declaration);
            auto* root = doc.NewElement("newspapers");
            doc.InsertEndChild(root);
            for(int i = 0; i < this->newspapers.size(); i++) {
                auto* newspaper = doc.NewElement("newspaper");
                newspaper->SetAttribute("index", this->newspapers[i].index);
                auto* title = doc.NewElement("title");
                title->SetText(this->newspapers[i].title.c_str());
                newspaper->InsertEndChild(title);
                auto* topic = doc.NewElement("topic");
                topic->SetText(std::to_string(this->newspapers[i].topic).c_str());
                newspaper->InsertEndChild(topic);
                auto* editor = doc.NewElement("editor");
                editor->SetText(this->newspapers[i].editor.c_str());
                newspaper->InsertEndChild(editor);
                root->InsertEndChild(newspaper);
            }
            doc.SaveFile((base/lg/nw).string().c_str());
        }
        {
            XMLDocument doc;
            auto* declaration = doc.NewDeclaration();
            doc.InsertFirstChild(declaration);
            auto* root = doc.NewElement("printing-houses");
            doc.InsertEndChild(root);
            for(int i = 0; i < this->printing_houses.size(); i++) {
                auto* printing_house = doc.NewElement("printing-house");
                printing_house->SetAttribute("id", this->printing_houses[i].id);

                auto* title = doc.NewElement("title");
                title->SetText(this->printing_houses[i].title.c_str());
                printing_house->InsertEndChild(title);

                auto* address = doc.NewElement("address");
                address->SetText(this->printing_houses[i].address.c_str());
                printing_house->InsertEndChild(address);

                auto* boss = doc.NewElement("boss");
                boss->SetText(this->printing_houses[i].boss.c_str());
                printing_house->InsertEndChild(boss);

                auto* newspapers = doc.NewElement("newspapers");
                for (const auto& [key, value] : this->printing_houses[i].newspapers) {
                    auto* newspaper = doc.NewElement("newspaper");
                    newspaper->SetAttribute("index", key);

                    auto* circulation = doc.NewElement("circulation");
                    circulation->SetText(std::to_string(std::get<0>(value)).c_str());
                    newspaper->InsertEndChild(circulation);

                    auto* price = doc.NewElement("price");
                    price->SetText(std::to_string(std::get<1>(value)).c_str());
                    newspaper->InsertEndChild(price);


                    newspapers->InsertEndChild(newspaper);
                }
                printing_house->InsertEndChild(newspapers);

                root->InsertEndChild(printing_house);
            }
            doc.SaveFile((base/lg/ph).string().c_str());
        }
        {
            XMLDocument doc;
            auto* declaration = doc.NewDeclaration();
            doc.InsertFirstChild(declaration);
            auto* root = doc.NewElement("post-offices");
            doc.InsertEndChild(root);
            for(int i = 0; i < this->post_offices.size(); i++) {
                auto* post_office = doc.NewElement("post-office");
                post_office->SetAttribute("number", this->post_offices[i].number);

               
                auto* address = doc.NewElement("address");
                address->SetText(this->post_offices[i].address.c_str());
                post_office->InsertEndChild(address);

                auto* region = doc.NewElement("region");
                region->SetText(this->post_offices[i].region.c_str());
                post_office->InsertEndChild(region);

                auto* printing_houses = doc.NewElement("printing-houses");
                for (const auto& [key, value] : this->post_offices[i].printing_houses) {
                    auto* printing_house = doc.NewElement("printing-house");
                    printing_house->SetAttribute("id", std::get<0>(key));
                    printing_house->SetAttribute("index", std::get<1>(key));

                    auto* count = doc.NewElement("count");
                    count->SetText(std::to_string(std::get<0>(value)).c_str());
                    printing_house->InsertEndChild(count);

                    auto* price = doc.NewElement("price");
                    price->SetText(std::to_string(std::get<1>(value)).c_str());
                    printing_house->InsertEndChild(price);

                    printing_houses->InsertEndChild(printing_house);
                }
                post_office->InsertEndChild(printing_houses);
                root->InsertEndChild(post_office);
            }
            doc.SaveFile((base/lg/po).string().c_str());
        }
    
    }

    void load_user_data(const std::string& user) {
        std::filesystem::path lg{user};

        if(!std::filesystem::exists(base/lg/nw)) {
            XMLDocument doc;
            auto* declaration = doc.NewDeclaration();
            doc.InsertFirstChild(declaration);
            auto* root = doc.NewElement("newspapers");
            doc.InsertEndChild(root);
            doc.SaveFile((base/lg/nw).string().c_str());
        };
        if(!std::filesystem::exists(base/lg/ph)) {
            XMLDocument doc;
            auto* declaration = doc.NewDeclaration();
            doc.InsertFirstChild(declaration);
            auto* root = doc.NewElement("printing-houses");
            doc.InsertEndChild(root);
            doc.SaveFile((base/lg/ph).string().c_str());
        };
        if(!std::filesystem::exists(base/lg/po)) {
            XMLDocument doc;
            auto* declaration = doc.NewDeclaration();
            doc.InsertFirstChild(declaration);
            auto* root = doc.NewElement("post-offices");
            doc.InsertEndChild(root);
            doc.SaveFile((base/lg/po).string().c_str());
        };

        {
            XMLDocument doc;
            doc.LoadFile((base/lg/nw).string().c_str());
            newspapers.clear();

            auto* root = doc.FirstChildElement("newspapers");
            auto* newspaper = root->FirstChildElement("newspaper");
            while (newspaper != NULL) {
                std::string title;
                std::string editor;
                int index {0};
                int topic {0};

                newspaper->QueryIntAttribute("index", &index);
                auto* t_title = newspaper->FirstChildElement("title")->GetText();
                if (t_title) {title = t_title;}
                newspaper->FirstChildElement("topic")->QueryIntText(&topic);
                auto* t_editor = newspaper->FirstChildElement("editor")->GetText();
                if (t_editor) {editor = t_editor;}
                newspapers.push_back(Newspaper(title, (Newspaper::topics)topic, index, editor));
                newspaper = newspaper->NextSiblingElement("newspaper");
            }
        }
        {
            XMLDocument doc;
            doc.LoadFile((base/lg/ph).string().c_str());
            printing_houses.clear();
         
            auto* root = doc.FirstChildElement("printing-houses");
            auto* printing_house = root->FirstChildElement("printing-house");
            while (printing_house != NULL) {
                std::string title;
                std::string address;
                std::string director;
                int id {0};
                printing_house->QueryIntAttribute("id", &id);
                auto* t_title = printing_house->FirstChildElement("title")->GetText();
                if (t_title) {title = t_title;}
                auto* t_address = printing_house->FirstChildElement("address")->GetText();
                if (t_address) {address = t_address;}
                auto* t_director = printing_house->FirstChildElement("boss")->GetText();
                if (t_director) {director = t_director;}
                
                
                PrintingHouse t = PrintingHouse(title, address, director, id);
                auto* newspapers = printing_house->FirstChildElement("newspapers");
                if(newspapers) {
                    auto* newspaper = newspapers->FirstChildElement("newspaper");
                    while (newspaper != NULL) {
                        int circulation{0};
                        float price{0};
                        int index {0};
                        newspaper->QueryIntAttribute("index", &index);
                        
                        newspaper->FirstChildElement("circulation")->QueryIntText(&circulation);
                        newspaper->FirstChildElement("price")->QueryFloatText(&price);
                        
                        t.newspapers[index] = std::tuple<int, float>(circulation, price);
                       
                        newspaper = newspaper->NextSiblingElement("newspaper");
                    }
                }

                printing_houses.push_back(t);
                printing_house = printing_house->NextSiblingElement("printing-house");
            }
            
        }
        {
            XMLDocument doc;
            doc.LoadFile((base/lg/po).string().c_str());
            post_offices.clear();

            auto* root = doc.FirstChildElement("post-offices");
            auto* post_office = root->FirstChildElement("post-office");
            while (post_office != NULL) {
                std::string address;
                std::string region;
                int number {0};
                post_office->QueryIntAttribute("number", &number);
                auto* t_address = post_office->FirstChildElement("address")->GetText();
                if (t_address) {address = t_address;}
                auto* t_region = post_office->FirstChildElement("region")->GetText();
                if (t_region) {region = t_region;}

                PostOffice t = PostOffice(number, address, region);
            
                auto* printing_houses = post_office->FirstChildElement("printing-houses");
                auto* printing_house = printing_houses->FirstChildElement("printing-house");
                while (printing_house != NULL) {
                    
                    int id {0};
                    int index {0};
                    printing_house->QueryIntAttribute("id", &id);
                    printing_house->QueryIntAttribute("index", &index);
                    int count {0};
                    float price {0};
                    printing_house->FirstChildElement("count")->QueryIntText(&count);
                    printing_house->FirstChildElement("price")->QueryFloatText(&price);
                    std::cout << 1;
                    t.add_order(std::tuple<int, int>(id, index), std::tuple<int, float>(count, price));
                    printing_house = printing_house->NextSiblingElement("printing-house");
                }
                post_offices.push_back(t);
                post_office = post_office->NextSiblingElement("post-office");
            }
        }
}

    Newspaper* get_newspaper_by_index(int indx) {
        for(int i = 0; i < this->newspapers.size(); i++) {
            if(this->newspapers[i].index == indx)
                return &this->newspapers[i];
        }
        return nullptr;
    }

    int get_location_newspaper_by_index(int indx) {
        for(int i = 0; i < this->newspapers.size(); i++) {
            if(this->newspapers[i].index == indx)
                return i;
        }
        return -1;
    }

    PrintingHouse* get_printing_house_by_index(int id) {
        for(int i = 0; i < this->printing_houses.size(); i++) {
            if(this->printing_houses[i].id == id)
                return &this->printing_houses[i];
        }
        return nullptr;
    }

    int get_location_printing_house_by_index(int id) {
        for(int i = 0; i < this->printing_houses.size(); i++) {
            if(this->printing_houses[i].id == id)
                return i;
        }
        return -1;
    }

    PostOffice* get_post_office_by_index(int number) {
        for(int i = 0; i < this->post_offices.size(); i++) {
            if(this->post_offices[i].number == number)
                return &this->post_offices[i];
        }
        return nullptr;
    }

    int get_location_post_office_by_index(int number) {
        for(int i = 0; i < this->post_offices.size(); i++) {
            if(this->post_offices[i].number == number)
                return i;
        }
        return -1;
    }

};



class Main : public QMainWindow {
    enum StatusMassage{okk, info, error};
    Ui::MainWindow ui;
    BD bd;
    unsigned char cur_lan{0};
    
    
public:

    Main(BD bd, QWidget* parent=nullptr) : QMainWindow(parent) {
        ui.setupUi(this);
        this->bd = bd;

        work_space_disable();

        std::vector<std::string> t = bd.get_users();
        if (t.size())
            for(int i=0; i < t.size(); i++)
                ui.logins->addItem(t[i].c_str());

        //save 
        connect(ui.save, &QPushButton::clicked, this, &Main::on_save);
        connect(ui.load, &QPushButton::clicked, this, &Main::on_load);
        connect(ui.change_language, &QPushButton::clicked, this, &Main::on_change_language);

        //tab home
        connect(ui.user_create, &QPushButton::clicked, this, &Main::on_user_create);
        connect(ui.spaces, &QTabWidget::tabBarClicked, this, &Main::on_tab_clicked);
        connect(ui.spaces, &QTabWidget::currentChanged, this, &Main::update_message);
        connect(ui.in, &QPushButton::clicked, this, &Main::on_in);
        connect(ui.out, &QPushButton::clicked, this, &Main::on_out);
        connect(ui.del, &QPushButton::clicked, this, &Main::on_del);

        //tab newspapers
        connect(ui.newspaper_add, &QPushButton::clicked, this, &Main::on_newspaper_add);
        connect(ui.newspapers_list, &QListWidget::itemClicked, this, &Main::change_newspaper);
        connect(ui.newspapers_list, &QListWidget::itemDoubleClicked, this, &Main::clear_newspaper_fields);
        connect(ui.newspaper_save, &QPushButton::clicked, this, &Main::on_newspaper_save);
        connect(ui.newspaper_del, &QPushButton::clicked, this, &Main::on_newspaper_del);

        //tab printing house
        connect(ui.printing_house_add, &QPushButton::clicked, this, &Main::on_printing_house_add);
        connect(ui.printing_house_list, &QListWidget::itemClicked, this, &Main::change_printing_house);
        connect(ui.printing_house_list, &QListWidget::itemDoubleClicked, this, &Main::clear_printing_house_fields);
        connect(ui.printing_house_save, &QPushButton::clicked, this, &Main::on_printing_house_save);
        connect(ui.printing_house_del, &QPushButton::clicked, this, &Main::on_printing_house_del);
        connect(ui.newspapers_per_printing_house_add, &QPushButton::clicked, this, &Main::on_newspapers_per_printing_house_add);
        connect(ui.newspapers_per_printing_house_list, &QListWidget::itemClicked, this, &Main::change_newspapers_per_printing_house);
        connect(ui.newspapers_per_printing_house_list, &QListWidget::itemDoubleClicked, this, &Main::clear_newspapers_per_printing_house_list_fields);
        connect(ui.newspapers_per_printing_house_del, &QPushButton::clicked, this, &Main::on_newspapers_per_printing_house_del);

        //tab post office
        connect(ui.post_office_add, &QPushButton::clicked, this, &Main::on_post_office_add);
        connect(ui.post_office_list, &QListWidget::itemClicked, this, &Main::change_post_office);
        connect(ui.post_office_list, &QListWidget::itemDoubleClicked, this, &Main::clear_post_office_fields);
        connect(ui.post_office_save, &QPushButton::clicked, this, &Main::on_post_office_save);
        connect(ui.post_office_del, &QPushButton::clicked, this, &Main::on_post_office_del);

        connect(ui.newspapers_per_post_office_add, &QPushButton::clicked, this, &Main::on_newspapers_per_post_office_add);
        connect(ui.printing_house_per_post_office, QOverload<int>::of(&QComboBox::activated), this, &Main::update_printing_house_per_current_post_office);
        connect(ui.newspapers_per_post_office_list, &QListWidget::itemDoubleClicked, this, &Main::clear_newspapers_per_post_office_list_fields);
        connect(ui.newspapers_per_post_office_list, &QListWidget::itemClicked, this, &Main::change_newspapers_per_post_office_list);

        // select 1
        connect(ui.selection1_update, &QPushButton::clicked, this, &Main::on_selection1_update);
        connect(ui.selection1_save, &QPushButton::clicked, this, &Main::on_selection1_save);
        
        // select 2
        connect(ui.selection2_update, &QPushButton::clicked, this, &Main::on_selection2_update);
        connect(ui.selection2_save, &QPushButton::clicked, this, &Main::on_selection2_save);
        // select 3
        connect(ui.selection3_update, &QPushButton::clicked, this, &Main::on_selection3_update);
        connect(ui.selection3_save, &QPushButton::clicked, this, &Main::on_selection3_save);
    }
    
    void on_save(bool) {
        bd.save_user_data(bd.cur_user);
    }
    
    void on_load(bool) {
        bd.load_user_data(bd.cur_user);

        update_newspaper_list();
        update_post_office_list();
        update_newspapers_per_printing_house();
        update_printing_house_list();
        update_printing_house_per_post_office();
        
    }

    void on_change_language(bool) {
        std::tuple<QString, QString> lan[3]{{QString("rus"),QString("Русский")}, {QString("jpn"),QString("日本語")}, {QString("eng"),QString("English")}};

        QTranslator translator;
        translator.load((bd.language_path/std::filesystem::path(std::get<0>(lan[cur_lan]).toStdString())).c_str());
        QApplication::installTranslator(&translator);
        ui.retranslateUi(this);

        ui.change_language->setText(std::get<1>(lan[cur_lan]));
        ui.cur_user->setText(bd.cur_user.c_str());

        cur_lan+=1;
        cur_lan %= sizeof(lan)/sizeof(*lan);
    };
 
    void on_user_create(bool) {
        update_message();

        if(!bd.is_free_login(ui.new_login->text().toStdString())) {
            get_massage(error, QCoreApplication::translate("Main", "The account login is busy", nullptr)); return;
        }
        if(!bd.is_valid_filename(ui.new_login->text().toStdString())) {
            get_massage(error, QCoreApplication::translate("Main", "The account login is incorrect", nullptr)); return;
        }
        bd.user_create(ui.new_login->text().toStdString());
        get_massage(okk, QCoreApplication::translate("Main", "Successful account creation", nullptr));
        ui.logins->addItem(ui.new_login->text());
        ui.new_login->clear();

    }

    void on_in(bool) {
        update_message();
        if(ui.logins->currentText().toStdString().size()) {
        ui.cur_user->setText(ui.logins->currentText());
        bd.cur_user = ui.logins->currentText().toStdString();
        on_load(1);
        get_massage(okk, QCoreApplication::translate("Main", "Successful account login", nullptr));
        work_space_enable();
        return;
        }
        get_massage(info, QCoreApplication::translate("Main", "Access is denied", nullptr));
    }

    void on_del(bool) {
        update_message();
        if(ui.logins->currentText().toStdString().size()) {
            if(ui.cur_user->text().toStdString() == ui.logins->currentText().toStdString())
                ui.cur_user->setText("");
        bd.user_del(ui.logins->currentText().toStdString());
        ui.logins->removeItem(ui.logins->currentIndex());
        get_massage(okk, QCoreApplication::translate("Main", "Successful account deletion", nullptr));
        return;
        }
        get_massage(error, QCoreApplication::translate("Main", "An empty account login field", nullptr));
    }

    void on_out(bool) {
        update_message();
        if(ui.cur_user->text().toStdString().size()) {
        ui.cur_user->setText("");
        bd.cur_user = "";
        get_massage(okk, QCoreApplication::translate("Main", "Successful account login", nullptr));
        work_space_disable();
        return;
        }
        get_massage(info, QCoreApplication::translate("Main", "An empty account login field", nullptr));
    }

    void get_massage(int status, const QString& massage) {
        ui.message->setStyleSheet("");
        ui.message->clear();
        switch (status)
        {
        case okk:
            ui.message->setStyleSheet("color: white; background-color: rgb(88, 129, 87); padding: 5px 5px;");
            ui.message->setText(massage);
            break;
        case error:
            ui.message->setStyleSheet("color: white; background-color: rgb(180, 56, 36); padding: 5px 5px;");
            ui.message->setText(massage);
            break;
            break;
        case info:
            ui.message->setStyleSheet("color: white; background-color: orange; padding: 5px 5px;");
            ui.message->setText(massage);
            break;
            break;
        
        default:
            break;
        }
    }

    void update_message() {
        ui.message->setStyleSheet("");
        ui.message->clear();
    }

    void on_tab_clicked(int index) {
        if(index == 4) selection1_update();
        if(index == 5) selection2_update();
        if(index == 6) selection3_update();
    }

    //tab newspapers
    void on_newspaper_add(bool) {
        if(ui.newspaper_title->text().isEmpty()) {
            get_massage(error, QString(QCoreApplication::translate("Main", "Empty title of the newspaper", nullptr)));
            return;
        }
        int indx = 0;
        if(bd.newspapers.size())
            indx = bd.newspapers.back().index+1;
        
        Newspaper t{ui.newspaper_title->text().toStdString(), (Newspaper::topics)ui.newspaper_topic->currentIndex(),
            indx, ui.newspaper_fio_editor->text().toStdString()};
        bd.add_newspapers(t);
        QListWidgetItem* tt = new QListWidgetItem(ui.newspaper_title->text());
        tt->setData(Qt::UserRole, indx);
        ui.newspapers_list->addItem(tt);


        clear_newspaper_fields();
        update_newspapers_per_printing_house();
        get_massage(okk, QString(QCoreApplication::translate("Main", "Successful newspaper addition", nullptr)));
    }

    void change_newspaper(QListWidgetItem *item) {
        int indx = ui.newspapers_list->currentRow();
        if(indx != -1 && bd.newspapers.size() > 0) {
        ui.newspaper_title->setText(QString(bd.newspapers[indx].title.c_str()));
        ui.newspaper_index->setValue(bd.newspapers[indx].index);
        ui.newspaper_fio_editor->setText(QString(bd.newspapers[indx].editor.c_str()));
        ui.newspaper_topic->setCurrentIndex(bd.newspapers[indx].topic);
        }
    }

    void on_newspaper_save(bool) {
        int indx = ui.newspapers_list->currentRow();
        if(indx != -1 && bd.newspapers.size() > 0) {
            if(ui.newspaper_title->text().isEmpty()) {get_massage(1, QString(QCoreApplication::translate("Main", "An empty newspaper title", nullptr))); return;}
            ui.newspapers_list->item(indx)->setText(ui.newspaper_title->text());
            bd.newspapers[indx].title = ui.newspaper_title->text().toStdString();
            bd.newspapers[indx].editor = ui.newspaper_fio_editor->text().toStdString();
            bd.newspapers[indx].topic = ui.newspaper_topic->currentIndex();
            clear_newspaper_fields();
            update_newspapers_per_printing_house();
        }
    }

    void clear_newspaper_fields() {
        ui.newspaper_title->setText("");
        ui.newspaper_index->setValue(0);
        ui.newspaper_fio_editor->setText("");
        ui.newspaper_topic->setCurrentIndex(0);
    }

    void on_newspaper_del(bool) {
        int indx = ui.newspapers_list->currentRow();
        if(indx != -1 && bd.newspapers.size() > 0) {
            bd.del_newspapers(indx);
            QListWidgetItem* item = ui.newspapers_list->takeItem(indx);
            if (item) delete item;
            clear_newspaper_fields();
            update_newspapers_per_printing_house();
        }
        return ;
    }

    //tab printing house
    void on_printing_house_add(bool) {
        if(ui.printing_house_title->text().isEmpty()) {
            get_massage(error, QString(QCoreApplication::translate("Main", "Empty title of the printing house", nullptr)));
            return;
        }

        int id = 0;
        if(bd.printing_houses.size())
            id = bd.printing_houses.back().id+1;
        
        PrintingHouse t{
            ui.printing_house_title->text().toStdString(), 
            ui.printing_house_address->text().toStdString(), 
            ui.printing_house_boss->text().toStdString(),
            id
        };

        bd.add_printing_house(t);
        QListWidgetItem* tt = new QListWidgetItem(ui.printing_house_title->text());
        tt->setData(Qt::UserRole, id);
        ui.printing_house_list->addItem(tt);

        clear_printing_house_fields();
        get_massage(okk, QString(QCoreApplication::translate("Main", "Successful addition of tiprographye", nullptr)));
    }

    void change_printing_house(QListWidgetItem *item) {
        if(ui.printing_house_list->currentRow()==-1) return;
        auto* order = bd.get_printing_house_by_index(ui.printing_house_list->currentItem()->data(Qt::UserRole).toInt());
        if(bd.printing_houses.size() > 0) {
            ui.printing_house_title->setText(QString(order->title.c_str()));
            ui.printing_house_address->setText(QString(order->address.c_str()));
            ui.printing_house_boss->setText(QString(order->boss.c_str()));
            
            update_newspapers_list_by_printing_house(bd.get_location_printing_house_by_index(order->id));
            
            
            clear_newspapers_per_printing_house_list_fields();
        }
    }

    void clear_printing_house_fields() {
        ui.printing_house_title->setText("");
        ui.printing_house_address->setText("");
        ui.printing_house_boss->setText("");
    }

    void on_printing_house_save(bool) {
        if(ui.printing_house_list->currentRow() == -1) return; 
        auto order = bd.get_printing_house_by_index(ui.printing_house_list->currentItem()->data(Qt::UserRole).value<int>());
        if(order && bd.printing_houses.size() > 0) {
        if(ui.printing_house_title->text().isEmpty()) {get_massage(1, QString(QCoreApplication::translate("Main", "Empty title of the printing house", nullptr))); return;}
        ui.printing_house_list->item(bd.get_location_printing_house_by_index(order->id))->setText(ui.printing_house_title->text());
        order->title = ui.printing_house_title->text().toStdString();
        order->address = ui.printing_house_address->text().toStdString();
        order->boss = ui.printing_house_boss->text().toStdString();
        clear_printing_house_fields();
        }
    }
// Удаление элементов из массива газет нужно реализовать
    void on_printing_house_del(bool) {
        if(ui.printing_house_list->currentRow() == -1) return; 
        auto* order = bd.get_printing_house_by_index(ui.printing_house_list->currentItem()->data(Qt::UserRole).toInt());
        if(order && bd.printing_houses.size() > 0) {
            int id = bd.get_location_printing_house_by_index(order->id);
            bd.del_printing_house(id);
            QListWidgetItem* item = ui.printing_house_list->takeItem(id);
            if (item) delete item;

            clear_printing_house_fields();
        }
        return ;
    }

    // block with newspapers
    void on_newspapers_per_printing_house_add() {
  
        if(!(ui.printing_house_list->currentRow() != -1 && bd.printing_houses.size() > 0)) {
            get_massage(error, QString(QCoreApplication::translate("Main", "Printing house is not selected", nullptr)));
            return;
        }
        if(!bd.newspapers.size()) {
            get_massage(error, QString(QCoreApplication::translate("Main", "There are no newspapers", nullptr)));
            return; 
        }

        auto* order = bd.get_printing_house_by_index(ui.printing_house_list->currentItem()->data(Qt::UserRole).toInt());
        int index = ui.newspapers_per_printing_house->currentData().toInt();

        QListWidgetItem *item = new QListWidgetItem(bd.get_newspaper_by_index(index)->title.c_str());

        item->setData(Qt::UserRole, order->id);
        item->setData(Qt::UserRole+1, index);

        if(order->newspapers.count(index) == 0)
        ui.newspapers_per_printing_house_list->addItem(item);


        std::tuple<int, float> circulation_and_price{ui.newspapers_count_per_printing_house->value(),ui.newspapers_price_per_printing_house->value()};
        order->add_newspaper(
            index,
            circulation_and_price
        );

        clear_newspapers_per_printing_house_list_fields();
        get_massage(okk, QString(QCoreApplication::translate("Main", "Successful addition of the newspaper to the printing house", nullptr)));
    }

    void clear_newspapers_per_printing_house_list_fields() {
        ui.newspapers_per_printing_house->setCurrentIndex(-1);
        ui.newspapers_count_per_printing_house->setValue(0);
        ui.newspapers_price_per_printing_house->setValue(0);
    }
   
    void change_newspapers_per_printing_house(QListWidgetItem *item) {
        int indx = item->data(Qt::UserRole).toInt();
        int indx2 = item->data(Qt::UserRole+1).toInt();
        auto data = bd.get_printing_house_by_index(indx)->newspapers[indx2];
        ui.newspapers_count_per_printing_house->setValue(std::get<0>(data));
        ui.newspapers_price_per_printing_house->setValue(std::get<1>(data));
        ui.newspapers_per_printing_house->setCurrentIndex(bd.get_location_newspaper_by_index(indx2));

    }

    void on_newspapers_per_printing_house_del() {
        if(ui.printing_house_list->currentRow() == -1) return; 

        if(!(bd.printing_houses.size() > 0)) {
            get_massage(error, QString(QCoreApplication::translate("Main", "Printing house is not selected", nullptr)));
            return;
        }
        if(!bd.newspapers.size()) {
            get_massage(error, QString(QCoreApplication::translate("Main", "There are no newspapers", nullptr)));
            return; 
        }

        int indx2 = ui.newspapers_per_printing_house_list->currentRow();
        if(indx2 == -1) {
            get_massage(error, QString(QCoreApplication::translate("Main", "Newspaper NOT selected", nullptr)));
            return; 
        }

        int id = ui.newspapers_per_printing_house_list->currentItem()->data(Qt::UserRole).toInt();
        int index = ui.newspapers_per_printing_house_list->currentItem()->data(Qt::UserRole+1).value<int>();

        bd.get_printing_house_by_index(id)->del_newspaper(index);
        QListWidgetItem* item = ui.newspapers_per_printing_house_list->takeItem(ui.printing_house_list->currentRow());
        if (item) delete item;
        clear_newspapers_per_printing_house_list_fields();
        update_newspapers_list_by_printing_house(bd.get_location_printing_house_by_index(id));
        return ;
    }

    void update_newspapers_list_by_printing_house(int indx) {
        ui.newspapers_per_printing_house_list->clear();

        for (const auto& [indx2, value] : bd.printing_houses[indx].newspapers) {
            QListWidgetItem *item = new QListWidgetItem(bd.get_newspaper_by_index(indx2)->title.c_str());
            item->setData(Qt::UserRole, bd.printing_houses[indx].id);
            item->setData(Qt::UserRole+1, indx2);
            ui.newspapers_per_printing_house_list->addItem(item);
        }
    }
    
    //post office
    void on_post_office_add(bool) {
        if(!ui.post_office_number->value()) {
            get_massage(error, QString(QCoreApplication::translate("Main", "An empty post office number", nullptr)));
            return;
        }
        PostOffice t{
            ui.post_office_number->value(), 
            ui.post_office_address->text().toStdString(), 
            ui.post_office_region->text().toStdString()
        };
        
        bd.add_post_office(t);
        QListWidgetItem* tt = new QListWidgetItem(QString(QCoreApplication::translate("Main", "Post Office №")) + QString::number(ui.post_office_number->value(), 16, 0));
        tt->setData(Qt::UserRole, t.number);
        ui.post_office_list->addItem(tt);

        clear_post_office_fields();
        get_massage(okk, QString(QCoreApplication::translate("Main", "Successful addition of a post office", nullptr)));
    }
    
    void change_post_office(QListWidgetItem *item) {
        int indx = ui.post_office_list->currentItem()->data(Qt::UserRole).value<int>();

        auto* order = bd.get_post_office_by_index(indx);
        if(indx != -1 && bd.post_offices.size() > 0) {
            ui.post_office_number->setValue(order->number);
            ui.post_office_address->setText(order->address.c_str());
            ui.post_office_region->setText(order->region.c_str());
            ui.post_office_number->setEnabled(false);

            update_newspapers_per_post_office_list(bd.get_location_post_office_by_index(order->number));
        }
    }
    
    void clear_post_office_fields() {
        ui.post_office_number->setEnabled(true);
        ui.post_office_number->setValue(0);
        ui.post_office_address->setText("");
        ui.post_office_region->setText("");
    }

    void on_newspapers_per_post_office_add() {
        int indx = ui.post_office_list->currentRow();
        if(!(indx != -1 && bd.post_offices.size() > 0)) {
            get_massage(error, QString(QCoreApplication::translate("Main", "Post Office is not selected", nullptr)));
            return;
        }
        if(ui.printing_house_per_post_office->currentIndex() == -1) {
            get_massage(error, QString(QCoreApplication::translate("Main", "Not selected Printing House", nullptr)));
            return; 
        }
        if(ui.newspaper_per_post_office->currentIndex() == -1) {
            get_massage(error, QString(QCoreApplication::translate("Main", "Not selected Newspaper of this Printing House", nullptr)));
            return; 
        }

        int number = ui.post_office_list->currentItem()->data(Qt::UserRole).toInt();
        int id = ui.printing_house_per_post_office->currentData().toInt();
        int index = ui.newspaper_per_post_office->currentData().toInt();
        std::tuple<int, int> ID_PrintingHouse_IndexNewspaper{id, index};

        QListWidgetItem *item = new QListWidgetItem(ui.printing_house_per_post_office->currentText() + " > " + ui.newspaper_per_post_office->currentText());
        item->setData(Qt::UserRole, id);
        item->setData(Qt::UserRole+1, index);

        if(bd.post_offices[indx].printing_houses.count(ID_PrintingHouse_IndexNewspaper) == 0)
        ui.newspapers_per_post_office_list->addItem(item);


        std::tuple<int, float> count_and_price{ui.newspapers_per_post_office_count->value(),ui.newspapers_per_post_office_price->value()};
        bd.post_offices[indx].add_order(
            ID_PrintingHouse_IndexNewspaper, 
            count_and_price
        );

        clear_newspapers_per_post_office_list_fields();
        get_massage(okk, QString(QCoreApplication::translate("Main", "Successful addition of the order to the Post Office", nullptr)));
    }

    void clear_newspapers_per_post_office_list_fields() {
        ui.newspapers_per_post_office_count->setValue(0);
        ui.newspapers_per_post_office_price->setValue(0);
        ui.printing_house_per_post_office->setCurrentIndex(-1);
        ui.newspaper_per_post_office->setCurrentIndex(-1);
    }
    
    void change_newspapers_per_post_office_list(QListWidgetItem *item) {
        int id = item->data(Qt::UserRole).value<int>();
        int index = item->data(Qt::UserRole+1).value<int>();
        int indx = ui.post_office_list->currentItem()->data(Qt::UserRole).value<int>();

        auto data = bd.get_post_office_by_index(indx)->printing_houses[std::tuple<int, int>(id, index)];
        ui.newspapers_per_post_office_count->setValue(std::get<0>(data));
        ui.newspapers_per_post_office_price->setValue(std::get<1>(data));

        
        update_printing_house_per_post_office();
        update_printing_house_per_current_post_office(ui.printing_house_per_post_office->currentIndex());
     
        
        ui.printing_house_per_post_office->setCurrentIndex(bd.get_location_printing_house_by_index(id));
        ui.newspaper_per_post_office->setCurrentIndex(bd.get_location_newspaper_by_index(index));
  
  
    }

    void update_newspapers_per_post_office_list(int number) {
        ui.newspapers_per_post_office_list->clear();

        auto order = &bd.post_offices[number];

        for (const auto& [indx2, value] : order->printing_houses) {
            // std::map<std::tuple<int, int>, std::tuple<int, float>>
            auto printing_house = bd.get_printing_house_by_index(std::get<0>(indx2));
            auto newspaper = bd.get_newspaper_by_index(std::get<1>(indx2));
            
            QListWidgetItem *item = new QListWidgetItem(QString(printing_house->title.c_str()) + " > " + QString(newspaper->title.c_str()) );
    
            item->setData(Qt::UserRole, printing_house->id);
            item->setData(Qt::UserRole+1, newspaper->index);
            ui.newspapers_per_post_office_list->addItem(item);

        }

    }

// Реализвать удаление списка типографий, которые подвязанны к этому офису
    void on_post_office_del() {
        int indx = ui.post_office_list->currentRow();
        if(indx != -1 && bd.post_offices.size() > 0) {
            bd.del_post_office(indx);
            QListWidgetItem* item = ui.post_office_list->takeItem(indx);
            if (item) delete item;
            clear_post_office_fields();
        }
    }

    void on_post_office_save() {
        int indx = ui.post_office_list->currentRow();
        if(indx != -1 && bd.post_offices.size() > 0) {
        bd.post_offices[indx].address = ui.post_office_address->text().toStdString();
        bd.post_offices[indx].region = ui.post_office_region->text().toStdString();
        clear_post_office_fields();
        }
    }

    //select 1 2 3
    void on_selection1_update(bool) {
        ui.selection1_table->clear();
        auto indx = ui.selection1->itemData(ui.selection1->currentIndex()).value<int>();
        if(bd.printing_houses.size() && indx!=-1) {
            std::vector<QString> head = {
                {QCoreApplication::translate("Main", "Subscription index", nullptr)},
                {QCoreApplication::translate("Main", "Title of Newspaper", nullptr)},
                {QCoreApplication::translate("Main", "Topic", nullptr)},
                {QCoreApplication::translate("Main", "Editor", nullptr)},
                {QCoreApplication::translate("Main", "Circulation", nullptr)},
                {QCoreApplication::translate("Main", "Price", nullptr)}};

            auto order_bd = bd.get_printing_house_by_index(indx);
            ui.selection1_table->setRowCount(order_bd->newspapers.size()+1);
            ui.selection1_table->setColumnCount(head.size());
            ui.selection1_table->horizontalHeader()->setVisible(false);
            ui.selection1_table->verticalHeader()->setVisible(false);
            

            for(int i = 0; i < head.size(); i++) {
                ui.selection1_table->horizontalHeader()->setSectionResizeMode(i, QHeaderView::Stretch);
                QTableWidgetItem* t = new QTableWidgetItem();
                t->setText(head[i]);
                ui.selection1_table->setItem(0, i, t);
            }

            int order{1};
            for(auto& [key, value] : order_bd->newspapers) {
                auto ord = *bd.get_newspaper_by_index(key);
                QTableWidgetItem* t0 = new QTableWidgetItem();
                t0->setText(QString::number(ord.index, 10, 0));
                t0->setFlags(t0->flags() & ~Qt::ItemIsEditable);
                ui.selection1_table->setItem(order, 0, t0);
                QTableWidgetItem* t1 = new QTableWidgetItem();
                t1->setText(QString(ord.title.c_str()));
                t1->setFlags(t1->flags() & ~Qt::ItemIsEditable);
                ui.selection1_table->setItem(order, 1, t1);
                QTableWidgetItem* t2 = new QTableWidgetItem();
                t2->setText(ui.newspaper_topic->itemText(ord.topic));
                t2->setFlags(t2->flags() & ~Qt::ItemIsEditable);
                ui.selection1_table->setItem(order, 2, t2);
                QTableWidgetItem* t3 = new QTableWidgetItem();
                t3->setText(QString(ord.editor.c_str()));
                t3->setFlags(t3->flags() & ~Qt::ItemIsEditable);
                ui.selection1_table->setItem(order, 3, t3);
                QTableWidgetItem* t4 = new QTableWidgetItem();
                t4->setText(QString::number(std::get<0>(value), 10,0));
                t4->setFlags(t4->flags() & ~Qt::ItemIsEditable);
                ui.selection1_table->setItem(order, 4, t4);
                QTableWidgetItem* t5 = new QTableWidgetItem();
                t5->setText(QString::number(std::get<1>(value), 10,2));
                t5->setFlags(t5->flags() & ~Qt::ItemIsEditable);
                ui.selection1_table->setItem(order, 5, t5);
                order++;
            }
        }
        
    }
    
    void on_selection1_save(bool) {
        size_t M = ui.selection1_table->rowCount();
        size_t N = ui.selection1_table->columnCount();
        std::ofstream file_out("select1.csv");
    
        for(int i = 0; i < M; i++) {
            for(int j = 0; j < N; j++) {
                file_out << ui.selection1_table->item(i, j)->text().toStdString();
                if(j+1 != N) file_out <<'\t';
            }
            file_out << '\n';
        }
        file_out.close();
    }

    void on_selection2_update(bool) {
        ui.selection2_table->clear();
        auto indx = ui.selection2->itemData(ui.selection2->currentIndex()).value<int>();
        
        if(bd.printing_houses.size() && indx!=-1) {
            std::vector<QString> head = {
                {QCoreApplication::translate("Main", "Title Of Printing House", nullptr)},
                {QCoreApplication::translate("Main", "Address", nullptr)},
                {QCoreApplication::translate("Main", "Director", nullptr)},
                {QCoreApplication::translate("Main", "Circulation", nullptr)},
                {QCoreApplication::translate("Main", "Price", nullptr)}};
            ui.selection2_table->setRowCount(1);
            ui.selection2_table->setColumnCount(head.size());
            ui.selection2_table->horizontalHeader()->setVisible(false);
            ui.selection2_table->verticalHeader()->setVisible(false);
            

            for(int i = 0; i < head.size(); i++) {
                ui.selection2_table->horizontalHeader()->setSectionResizeMode(i, QHeaderView::Stretch);
                QTableWidgetItem* t = new QTableWidgetItem();
                t->setText(head[i]);
                ui.selection2_table->setItem(0, i, t);
            }

            int order{1};
            for(int i = 0; i < bd.printing_houses.size(); i++) {
                if(bd.printing_houses[i].newspapers.find(indx) != bd.printing_houses[i].newspapers.end()) {
                    ui.selection2_table->setRowCount(ui.selection2_table->rowCount()+1);
                    QTableWidgetItem* t0 = new QTableWidgetItem();
                    t0->setText(bd.printing_houses[i].title.c_str());
                    t0->setFlags(t0->flags() & ~Qt::ItemIsEditable);
                    ui.selection2_table->setItem(order, 0, t0);
                    QTableWidgetItem* t1 = new QTableWidgetItem();
                    t1->setText(bd.printing_houses[i].address.c_str());
                    t1->setFlags(t1->flags() & ~Qt::ItemIsEditable);
                    ui.selection2_table->setItem(order, 1, t1);
                    QTableWidgetItem* t2 = new QTableWidgetItem();
                    t2->setText(bd.printing_houses[i].boss.c_str());
                    t2->setFlags(t2->flags() & ~Qt::ItemIsEditable);
                    ui.selection2_table->setItem(order, 2, t2);
                    QTableWidgetItem* t3 = new QTableWidgetItem();
                    t3->setText(QString::number(std::get<0>(bd.printing_houses[i].newspapers[indx]), 10,0));
                    t3->setFlags(t3->flags() & ~Qt::ItemIsEditable);
                    ui.selection2_table->setItem(order, 3, t3);
                    QTableWidgetItem* t4 = new QTableWidgetItem();
                    t4->setText(QString::number(std::get<1>(bd.printing_houses[i].newspapers[indx]), 10,2));
                    t4->setFlags(t4->flags() & ~Qt::ItemIsEditable);
                    ui.selection2_table->setItem(order, 4, t4);
                    order++;

                }
            }
        }
    }
    
    void on_selection2_save(bool) {
        size_t M = ui.selection2_table->rowCount();
        size_t N = ui.selection2_table->columnCount();
        std::ofstream file_out("select2.csv");
    
        for(int i = 0; i < M; i++) {
            for(int j = 0; j < N; j++) {
                file_out << ui.selection2_table->item(i, j)->text().toStdString();
                if(j+1 != N) file_out <<'\t';
            }
            file_out << '\n';
        }
        file_out.close();
    }

    void on_selection3_update(bool) {
        ui.selection3_table->clear();
        auto indx = ui.selection3->itemData(ui.selection3->currentIndex()).value<int>();
        
        if(bd.printing_houses.size() && indx!=-1) {
            std::vector<QString> head = {
                {QCoreApplication::translate("Main", "Title Of Printing House", nullptr)},
                {QCoreApplication::translate("Main", "Address", nullptr)}, 
                {QCoreApplication::translate("Main", "Director", nullptr)}, 
                {QCoreApplication::translate("Main", "Circulation", nullptr)}, 
                {QCoreApplication::translate("Main", "Price", nullptr)}};
            ui.selection3_table->setRowCount(1);
            ui.selection3_table->setColumnCount(head.size());
            ui.selection3_table->horizontalHeader()->setVisible(false);
            ui.selection3_table->verticalHeader()->setVisible(false);
            
            float total_cost{0.0};

            for(int i = 0; i < head.size(); i++) {
                ui.selection3_table->horizontalHeader()->setSectionResizeMode(i, QHeaderView::Stretch);
                QTableWidgetItem* t = new QTableWidgetItem();
                t->setText(head[i]);
                ui.selection3_table->setItem(0, i, t);
            }

            int order{1};
            for(int i = 0; i < bd.printing_houses.size(); i++) {
                if(bd.printing_houses[i].newspapers.find(indx) != bd.printing_houses[i].newspapers.end()) {
                    ui.selection3_table->setRowCount(ui.selection3_table->rowCount()+1);
                    QTableWidgetItem* t0 = new QTableWidgetItem();
                    t0->setText(bd.printing_houses[i].title.c_str());
                    t0->setFlags(t0->flags() & ~Qt::ItemIsEditable);
                    ui.selection3_table->setItem(order, 0, t0);
                    QTableWidgetItem* t1 = new QTableWidgetItem();
                    t1->setText(bd.printing_houses[i].address.c_str());
                    t1->setFlags(t1->flags() & ~Qt::ItemIsEditable);
                    ui.selection3_table->setItem(order, 1, t1);
                    QTableWidgetItem* t2 = new QTableWidgetItem();
                    t2->setText(bd.printing_houses[i].boss.c_str());
                    t2->setFlags(t2->flags() & ~Qt::ItemIsEditable);
                    ui.selection3_table->setItem(order, 2, t2);
                    QTableWidgetItem* t3 = new QTableWidgetItem();
                    t3->setText(QString::number(std::get<0>(bd.printing_houses[i].newspapers[indx]), 10,0));
                    t3->setFlags(t3->flags() & ~Qt::ItemIsEditable);
                    ui.selection3_table->setItem(order, 3, t3);
                    QTableWidgetItem* t4 = new QTableWidgetItem();
                    t4->setText(QString::number(std::get<1>(bd.printing_houses[i].newspapers[indx]), 10,2));
                    t4->setFlags(t4->flags() & ~Qt::ItemIsEditable);
                    ui.selection3_table->setItem(order, 4, t4);
                    total_cost += std::get<0>(bd.printing_houses[i].newspapers[indx]) * std::get<1>(bd.printing_houses[i].newspapers[indx]);
                    order++;
                }
            }
            ui.total_cost->setText(QString::number(total_cost, 10, 2));
        }
    }
    
    void on_selection3_save(bool) {
        size_t M = ui.selection3_table->rowCount();
        size_t N = ui.selection3_table->columnCount();
        std::ofstream file_out("select3.csv");
    
        for(int i = 0; i < M; i++) {
            for(int j = 0; j < N; j++) {
                file_out << ui.selection3_table->item(i, j)->text().toStdString();
                if(j+1 != N) file_out <<'\t';
            }
            file_out << '\n';
        }
        file_out.close();
    }

    void selection1_update() {
        ui.selection1->clear();
        for(int i = 0; i < bd.printing_houses.size(); i++) {
            ui.selection1->addItem(bd.printing_houses[i].title.c_str(), bd.printing_houses[i].id);
        }
    }
    
    void selection2_update() {
        ui.selection2->clear();
        for(int i = 0; i < bd.newspapers.size(); i++) {
            ui.selection2->addItem(bd.newspapers[i].title.c_str(), bd.newspapers[i].index);
        }
    }
    
    void selection3_update() {
        ui.selection3->clear();
        for(int i = 0; i < bd.newspapers.size(); i++) {
            ui.selection3->addItem(bd.newspapers[i].title.c_str(), bd.newspapers[i].index);
        }
    }


    //etc
    void work_space_enable() {
        ui.save->setEnabled(true);
        ui.load->setEnabled(true);
        for(int i = 1; i < ui.spaces->count(); i++)
            ui.spaces->setTabEnabled(i,true);
    }

    void work_space_disable() {
        ui.save->setEnabled(false);
        ui.load->setEnabled(false);
        for(int i = 1; i < ui.spaces->count(); i++)
            ui.spaces->setTabEnabled(i,false);
    }


    // update element's content
    void update_newspaper_list() {
        ui.newspapers_list->clear();
        for(int i = 0; i < bd.newspapers.size(); i++) {
            QListWidgetItem* tt = new QListWidgetItem(bd.newspapers[i].title.c_str());
            tt->setData(Qt::UserRole, bd.newspapers[i].index);
            ui.newspapers_list->addItem(tt);
        }
    }

    void update_newspapers_per_printing_house() {
        ui.newspapers_per_printing_house->clear();
        for(int i = 0; i < bd.newspapers.size(); i++) {
            ui.newspapers_per_printing_house->addItem(bd.newspapers[i].title.c_str(), bd.newspapers[i].index);
        }
    }

    void update_printing_house_per_post_office() {
        ui.printing_house_per_post_office->clear();
        for(int i = 0; i < bd.printing_houses.size(); i++) {
            ui.printing_house_per_post_office->addItem(bd.printing_houses[i].title.c_str(), bd.printing_houses[i].id);
        }
    }

    void update_printing_house_per_current_post_office(int id) {
        ui.newspaper_per_post_office->clear();
        auto indx1 = ui.printing_house_per_post_office->itemData(id).value<int>();

        for(const auto& [i, value] : bd.get_printing_house_by_index(indx1)->newspapers) {
            Newspaper* t = bd.get_newspaper_by_index(i);
            ui.newspaper_per_post_office->addItem(QString(t->title.c_str()), t->index);
        }
        

    }


    void update_printing_house_list() {
        ui.printing_house_list->clear();
        for(int i = 0; i < bd.printing_houses.size(); i++) {
            QListWidgetItem* tt = new QListWidgetItem(bd.printing_houses[i].title.c_str());
            tt->setData(Qt::UserRole, bd.printing_houses[i].id);
            ui.printing_house_list->addItem(tt);
        }
    }

    void update_post_office_list() {
        ui.post_office_list->clear();
        for(int i = 0; i < bd.post_offices.size(); i++) {
            QListWidgetItem* tt = new QListWidgetItem(QString(QCoreApplication::translate("Main", "Post Office №")) + QString::number(bd.post_offices[i].number,10,0));
            tt->setData(Qt::UserRole, bd.post_offices[i].number);
            ui.post_office_list->addItem(tt);
        }
    }
};


int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    BD bd;
    bd.load();

    QTranslator translator;
    translator.load("rus");
    app.installTranslator(&translator);

    Main widget(bd);
    widget.show();
    return app.exec();
}

