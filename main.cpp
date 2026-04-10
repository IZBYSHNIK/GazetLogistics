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
    unsigned int index{0};
    std::string editor;
    
    Newspaper() {};
    Newspaper(const std::string& title, topics topic, unsigned int index, const std::string& editor) : title{title}, topic{topic}, index{index}, editor{editor} {};
};

class PrintingHouse {
    public:
    unsigned int id{0};
    std::string title;
    std::string address;
    std::string boss;
    // index from Newspapers and circulation and price for one
    std::map<unsigned int, std::tuple<unsigned int, unsigned int>> newspapers;
    
    PrintingHouse() {};
    
    PrintingHouse(const std::string& title, const std::string& address, const std::string& boss, unsigned int id=0) : title{title}, address{address}, boss{boss}, id{id} {};

    bool add_newspaper(unsigned int newspaper_index, std::tuple<unsigned int, unsigned int> circulation_and_price) {
        newspapers[newspaper_index] = circulation_and_price;
        return 0;
    }
    bool del_newspaper(unsigned int newspaper_index) {
        newspapers.erase(newspaper_index);
        return 0;
    }
};

class PostOffice {
    public:
    unsigned int number{0};
    std::string address;
    std::string region;
    std::map<unsigned int, std::tuple<unsigned int, unsigned int>> printing_houses;

    PostOffice() {};

    PostOffice(unsigned int number, const std::string& address, const std::string& region) : number{number}, address{address}, region{region} {};
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

    bool del_post_office(unsigned int indx) {
        auto it = post_offices.begin() + indx; 
        post_offices.erase(it);
        return 0;
    }

    bool del_newspapers(unsigned int indx) {
        // indx for vector, index not from Newspaper
        auto it = newspapers.begin() + indx; 
        newspapers.erase(it);
        return 0;
    }

    bool del_printing_house(unsigned int indx) {
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
                // for (const auto& [key, value] : this->printing_houses[i].newspapers) {
                //     auto* newspaper = doc.NewElement("newspaper");
                //     newspaper->SetAttribute("index", key);

                //     auto* circulation = doc.NewElement("circulation");
                //     circulation->SetText(std::to_string(std::get<0>(value)).c_str());
                //     newspaper->InsertEndChild(circulation);

                //     auto* price = doc.NewElement("price");
                //     price->SetText(std::to_string(std::get<1>(value)).c_str());
                //     newspaper->InsertEndChild(price);


                //     newspapers->InsertEndChild(newspaper);
                // }
                post_office->InsertEndChild(printing_houses);

                root->InsertEndChild(post_office);
            }
            doc.SaveFile((base/lg/po).string().c_str());
        }
    }

    void load_user_data(const std::string& user) {
       
    }

    const Newspaper& get_newspaper_by_index(unsigned int indx) {
        for(int i = 0; i < this->newspapers.size(); i++) {
            if(this->newspapers[i].index == indx)
                return this->newspapers[i];
        }
        return Newspaper();
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
        // connect(ui.newspapers_per_post_office_list, &QListWidget::itemClicked, this, &Main::change_newspapers_per_printing_house);
        // connect(ui.newspapers_per_post_office_list, &QListWidget::itemDoubleClicked, this, &Main::clear_newspapers_per_printing_house_list_fields);
        // connect(ui.newspapers_per_post_office_del, &QPushButton::clicked, this, &Main::on_newspapers_per_printing_house_del);

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
    }

    void on_change_language(bool) {
        std::tuple<QString, QString> lan[]{{QString("rus"),QString("Русский")}, {QString("jpn"),QString("日本語")}, {QString("eng"),QString("English")}};

        QTranslator translator;
        translator.load((bd.language_path/std::filesystem::path(std::get<0>(lan[cur_lan]).toStdString())).c_str());
        QApplication::installTranslator(&translator);
        ui.retranslateUi(this);

        ui.change_language->setText(std::get<1>(lan[cur_lan]));
        ui.cur_user->setText(bd.cur_user.c_str());

        std::ofstream file_out("cur_lang");
        file_out << cur_lan;
        file_out.close();

        cur_lan+=1;
        cur_lan %= sizeof(lan)/sizeof(*lan);
    };
 
    void on_user_create(bool) {
        update_message();

        if(!bd.is_free_login(ui.new_login->text().toStdString())) {
            get_massage(error, tr("The account login is busy")); return;
        }
        if(!bd.is_valid_filename(ui.new_login->text().toStdString())) {
            get_massage(error, tr("The account login is incorrect")); return;
        }
        bd.user_create(ui.new_login->text().toStdString());
        get_massage(okk, tr("Successful account creation"));
        ui.logins->addItem(ui.new_login->text());
        ui.new_login->clear();

    }

    void on_in(bool) {
        update_message();
        if(ui.logins->currentText().toStdString().size()) {
        ui.cur_user->setText(ui.logins->currentText());
        bd.cur_user = ui.logins->currentText().toStdString();
        get_massage(okk, tr("Successful account login"));
        work_space_enable();
        return;
        }
        get_massage(info, tr("Access is denied"));
    }

    void on_del(bool) {
        update_message();
        if(ui.logins->currentText().toStdString().size()) {
            if(ui.cur_user->text().toStdString() == ui.logins->currentText().toStdString())
                ui.cur_user->setText("");
        bd.user_del(ui.logins->currentText().toStdString());
        ui.logins->removeItem(ui.logins->currentIndex());
        get_massage(okk, tr("Successful account deletion"));
        return;
        }
        get_massage(error, tr("An empty account login field"));
    }

    void on_out(bool) {
        update_message();
        if(ui.cur_user->text().toStdString().size()) {
        ui.cur_user->setText("");
        bd.cur_user = "";
        get_massage(okk, tr("Successful account login"));
        work_space_disable();
        return;
        }
        get_massage(info, tr("An empty account login field"));
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
            get_massage(error, QString(tr("Empty title of the newspaper")));
            return;
        }
        unsigned int indx = 0;
        if(bd.newspapers.size())
            indx = bd.newspapers.back().index+1;
        
        Newspaper t{ui.newspaper_title->text().toStdString(), (Newspaper::topics)ui.newspaper_topic->currentIndex(),
            indx, ui.newspaper_fio_editor->text().toStdString()};
        bd.add_newspapers(t);
        ui.newspapers_list->addItem(ui.newspaper_title->text());


        clear_newspaper_fields();
        update_newspapers_per_printing_house();
        get_massage(okk, QString(tr("Successful newspaper addition")));
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
            if(ui.newspaper_title->text().isEmpty()) {get_massage(1, QString(tr("An empty newspaper title"))); return;}
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
            get_massage(error, QString(tr("Empty title of the printing house")));
            return;
        }

        unsigned int id = 0;
        if(bd.printing_houses.size())
            id = bd.printing_houses.back().id+1;
        
        PrintingHouse t{
            ui.printing_house_title->text().toStdString(), 
            ui.printing_house_address->text().toStdString(), 
            ui.printing_house_boss->text().toStdString(),
            id
        };

        bd.add_printing_house(t);
        ui.printing_house_list->addItem(ui.printing_house_title->text());

        clear_printing_house_fields();
        get_massage(okk, QString(tr("Successful addition of tiprographye")));
    }

    void change_printing_house(QListWidgetItem *item) {
        int indx = ui.printing_house_list->currentRow();
        if(indx != -1 && bd.printing_houses.size() > 0) {
            ui.printing_house_title->setText(QString(bd.printing_houses[indx].title.c_str()));
            ui.printing_house_address->setText(QString(bd.printing_houses[indx].address.c_str()));
            ui.printing_house_boss->setText(QString(bd.printing_houses[indx].boss.c_str()));
            update_newspapers_list_by_printing_house(indx);
            clear_newspapers_per_printing_house_list_fields();
        }
    }

    void clear_printing_house_fields() {
        ui.printing_house_title->setText("");
        ui.printing_house_address->setText("");
        ui.printing_house_boss->setText("");
    }

    void on_printing_house_save(bool) {
        int indx = ui.printing_house_list->currentRow();
        if(indx != -1 && bd.printing_houses.size() > 0) {
        if(ui.printing_house_title->text().isEmpty()) {get_massage(1, QString(tr("Empty title of the printing house"))); return;}
        ui.printing_house_list->item(indx)->setText(ui.printing_house_title->text());
        bd.printing_houses[indx].title = ui.printing_house_title->text().toStdString();
        bd.printing_houses[indx].address = ui.printing_house_address->text().toStdString();
        bd.printing_houses[indx].boss = ui.printing_house_boss->text().toStdString();
        clear_printing_house_fields();
        }
    }

// Удаление элементов из массива газет нужно реализовать
    void on_printing_house_del(bool) {
        int indx = ui.printing_house_list->currentRow();
        if(indx != -1 && bd.printing_houses.size() > 0) {
            bd.del_printing_house(indx);
            QListWidgetItem* item = ui.printing_house_list->takeItem(indx);
            if (item) delete item;

            clear_printing_house_fields();
        }
        return ;
    }

    // block with newspapers
    void on_newspapers_per_printing_house_add() {
        int indx = ui.printing_house_list->currentRow();
        if(!(indx != -1 && bd.printing_houses.size() > 0)) {
            get_massage(error, QString(tr("Printing house is not selected")));
            return;
        }
        if(!bd.newspapers.size()) {
            get_massage(error, QString(tr("There are no newspapers")));
            return; 
        }

       
        QListWidgetItem *item = new QListWidgetItem(bd.newspapers[ui.newspapers_per_printing_house->currentIndex()].title.c_str());
        item->setData(Qt::UserRole, bd.newspapers[ui.newspapers_per_printing_house->currentIndex()].index);
        // item->setData(Qt::UserRole+1, bd.newspapers[ui.newspapers_per_printing_house->currentIndex()].title);
        if(bd.printing_houses[indx].newspapers.count(bd.newspapers[ui.newspapers_per_printing_house->currentIndex()].index) == 0)
        ui.newspapers_per_printing_house_list->addItem(item);


        std::tuple<unsigned int, unsigned int> circulation_and_price{ui.newspapers_count_per_printing_house->value(),ui.newspapers_price_per_printing_house->value()};
        bd.printing_houses[indx].add_newspaper(
            bd.newspapers[ui.newspapers_per_printing_house->currentIndex()].index,
            circulation_and_price
        );

        clear_newspapers_per_printing_house_list_fields();
        get_massage(okk, QString(tr("Successful addition of the newspaper to the printing house")));
    }

    void clear_newspapers_per_printing_house_list_fields() {
        ui.newspapers_per_printing_house->setCurrentIndex(0);
        ui.newspapers_count_per_printing_house->setValue(0);
        ui.newspapers_price_per_printing_house->setValue(0);
    }
   
    void change_newspapers_per_printing_house(QListWidgetItem *item) {
        unsigned int indx2 = item->data(Qt::UserRole).value<unsigned int>();
        
        int indx = ui.printing_house_list->currentRow();
        
        auto data = bd.printing_houses[indx].newspapers[indx2];
        ui.newspapers_count_per_printing_house->setValue(std::get<0>(data));
        ui.newspapers_price_per_printing_house->setValue(std::get<1>(data));
        
        ui.newspapers_per_printing_house->setCurrentIndex(indx2);

    }

    void on_newspapers_per_printing_house_del() {
        int indx = ui.printing_house_list->currentRow();
        if(!(indx != -1 && bd.printing_houses.size() > 0)) {
            get_massage(error, QString(tr("Printing house is not selected")));
            return;
        }
        if(!bd.newspapers.size()) {
            get_massage(error, QString(tr("There are no newspapers")));
            return; 
        }

        int indx2 = ui.newspapers_per_printing_house_list->currentRow();
        if(indx2 == -1) {
            get_massage(error, QString(tr("Newspaper NOT selected")));
            return; 
        }

        bd.printing_houses[indx].del_newspaper(ui.newspapers_per_printing_house_list->currentItem()->data(Qt::UserRole).value<unsigned int>());
        QListWidgetItem* item = ui.newspapers_per_printing_house_list->takeItem(indx);
        if (item) delete item;
        clear_newspapers_per_printing_house_list_fields();
        update_newspapers_list_by_printing_house(indx);
        std::cout << bd.printing_houses[indx].newspapers.size();
        return ;
    }
// вывод элементов массива вместо "---"
    void update_newspapers_list_by_printing_house(int indx) {
        ui.newspapers_per_printing_house_list->clear();

        for (const auto& [indx2, value] : bd.printing_houses[indx].newspapers) {
            QListWidgetItem *item = new QListWidgetItem("--------");
            item->setData(Qt::UserRole, indx2);
            ui.newspapers_per_printing_house_list->addItem(item);
        }
    }
    
    //post office
    void on_post_office_add(bool) {
        if(!ui.post_office_number->value()) {
            get_massage(error, QString(tr("An empty post office number")));
            return;
        }
        PostOffice t{
            (unsigned int)ui.post_office_number->value(), 
            ui.post_office_address->text().toStdString(), 
            ui.post_office_region->text().toStdString()
        };

        bd.add_post_office(t);
        ui.post_office_list->addItem(QString(tr("Post Office №")) + QString::number(ui.post_office_number->value(), 16, 0));

        clear_post_office_fields();
        get_massage(okk, QString(tr("Successful addition of a post office")));
    }
    
    void change_post_office(QListWidgetItem *item) {
        int indx = ui.post_office_list->currentRow();
        if(indx != -1 && bd.post_offices.size() > 0) {
            ui.post_office_number->setValue(bd.post_offices[indx].number);
            ui.post_office_address->setText(QString(bd.post_offices[indx].address.c_str()));
            ui.post_office_region->setText(QString(bd.post_offices[indx].region.c_str()));
            ui.post_office_number->setEnabled(false);
            // update_newspapers_list_by_printing_house(indx);
            // clear_newspapers_per_printing_house_list_fields();
        }
    }
    
    void clear_post_office_fields() {
        ui.post_office_number->setEnabled(true);
        ui.post_office_number->setValue(0);
        ui.post_office_address->setText("");
        ui.post_office_region->setText("");
    }

    void on_newspapers_per_post_office_add() {}
    

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
        auto indx = ui.selection1->itemData(ui.selection1->currentIndex()).value<unsigned int>();
        // if(!bd.printing_houses.size() || indx==-1)selection1_update();
        if(bd.printing_houses.size() && indx!=-1) {
            std::vector<QString> head = {{tr("Subscription index")}, {tr("Title of Newspaper")}, {tr("Topic")}, {tr("Editor")}, {tr("Circulation")}, {tr("Price")}};
            ui.selection1_table->setRowCount(bd.printing_houses[indx].newspapers.size()+1);
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
            for(auto& [key, value] : bd.printing_houses[indx].newspapers) {
                auto ord = bd.get_newspaper_by_index(key);
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
        auto indx = ui.selection2->itemData(ui.selection2->currentIndex()).value<unsigned int>();
        
        if(bd.printing_houses.size() && indx!=-1) {
            std::vector<QString> head = {{tr("Title Of Printing House")}, {tr("Address")}, {tr("Director")}, {tr("Circulation")}, {tr("Price")}};
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
        auto indx = ui.selection3->itemData(ui.selection3->currentIndex()).value<unsigned int>();
        
        if(bd.printing_houses.size() && indx!=-1) {
            std::vector<QString> head = {{tr("Title Of Printing House")}, {tr("Address")}, {tr("Director")}, {tr("Circulation")}, {tr("Price")}};
            ui.selection3_table->setRowCount(1);
            ui.selection3_table->setColumnCount(head.size());
            ui.selection3_table->horizontalHeader()->setVisible(false);
            ui.selection3_table->verticalHeader()->setVisible(false);
            
            long double total_cost{0.0};

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

    void update_newspapers_per_printing_house() {
        ui.newspapers_per_printing_house->clear();
        for(int i=0; i < bd.newspapers.size(); i++)
            ui.newspapers_per_printing_house->addItem(QString(bd.newspapers[i].title.c_str()));
    };
};




int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    BD bd;
    bd.load();

    Main widget(bd);

    

    //load languge
    // int cur{0};
    // std::ifstream file_out("cur_lang");
    // file_out >> cur;
    // file_out.close();
    // QTranslator translator;

    // translator.load(QString((bd.language_path/std::filesystem::path(std::get<0>(widget.lan[cur]))).toStdString()));
    // QApplication::installTranslator(&translator);
    
    
    widget.show();
    return app.exec();
}

