#include "aboutwindow.h"
#include "ui_aboutwindow.h"



aboutWindow::aboutWindow(QString version, QWidget *parent) : QDialog(parent), ui(new Ui::aboutWindow)
{
    ui->setupUi(this);

	ui->labelCurrent->setText(version);
	m_version = version.replace(".", "").toInt();

	QNetworkAccessManager *m = new QNetworkAccessManager();
	connect(m, SIGNAL(finished(QNetworkReply*)), this, SLOT(finished(QNetworkReply*)));
	m->get(QNetworkRequest(QUrl("http://imgbrd-grabber.googlecode.com/svn/trunk/VERSION")));

	resize(QSize(400, 190));
}

aboutWindow::~aboutWindow()
{
    delete ui;
}

void aboutWindow::finished(QNetworkReply *r)
{
	QString l = r->readAll();
	qDebug() << l;
	qDebug() << r->url();
	int latest = l.replace(".", "").toInt();
	if (latest <= m_version)
	{ ui->labelMessage->setText("<p style=\"font-size:8pt; font-style:italic; color:#808080;\">"+tr("Grabber est � jour")+"</p>"); }
	else
	{ ui->labelMessage->setText("<p style=\"font-size:8pt; font-style:italic; color:#808080;\">"+tr("Une nouvelle version est disponible : %1").arg(l)+"</p>"); }
}