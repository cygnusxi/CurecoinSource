#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include "clientmodel.h"

#include "version.h"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
    setMinimumSize(680, 380);
    ui->label_4->setObjectName("aboutBrandImage");
    ui->label->setObjectName("aboutTitleLabel");
    ui->versionLabel->setObjectName("aboutVersionBadge");
    ui->copyrightLabel->setObjectName("aboutCopyrightLabel");
    ui->label_2->setObjectName("aboutBodyCard");
}

void AboutDialog::setModel(ClientModel *model)
{
    if(model)
    {
        ui->versionLabel->setText(model->formatFullVersion());
    }
}

AboutDialog::~AboutDialog()
{
    delete ui;
}

void AboutDialog::on_buttonBox_accepted()
{
    close();
}
