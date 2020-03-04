#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlRecord>

typedef struct _cameraInfo
{
    QString id;
    QString name;
    QString IOPath;
    QString encodingType;
    QString cameraUser;
    QString cameraPassword;
    QString cameraType;
    QString     originalPath;
    QString     imgPath; 
    int     cameraNumber;
    int     state;
    int     usels;
}cameraInfo;


int main(int argc, char *argv[])
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");  
 
    db.setDatabaseName("/home/yskj/seven/zhgd-ai/sqlite/tileDB/zhgd-ai.db");
    if (!db.open())
    {
    	return 0;
    }
    QSqlQuery query;	//执行操作类对象
 
    //查询数据
    query.prepare("SELECT * FROM camera");
    query.exec();	//执行
	
    QSqlRecord recode = query.record();		//recode保存查询到一些内容信息，如表头、列数等等
    int column = recode.count();			//获取读取结果的列数
    printf("column count is %d\n", column);	
    QString s1 = recode.fieldName(0);		//获取第0列的列名
    QVector<cameraInfo> infoVect;
    while (query.next())
    {
	    cameraInfo tmp;
 	    tmp.name = query.value("name").toString();
	    tmp.originalPath = query.value("originalPath").toString();
	    tmp.imgPath = query.value("imgPath").toString();
	    tmp.encodingType = query.value("encodingType").toString();
	    tmp.cameraUser = query.value("cameraUser").toString();
	    tmp.cameraPassword = query.value("cameraPassword").toString();
	    infoVect.push_back(tmp);   //将查询到的内容存到testInfo向量中
    }
     for (int i=0; i<infoVect.size(); i++)    //打印输出
    {
	    qDebug() << infoVect[i].name << ":"	\
	             << infoVect[i].originalPath << ":"		\
		         << infoVect[i].imgPath << ":"		\
		         << infoVect[i].encodingType << ":" \
		         << infoVect[i].cameraUser \
		         << infoVect[i].cameraPassword;
	}
  while(1) {
	}
  return 0;
  //QApplication app(argc, argv);
  //int ret = app.exec();

  //window.hide();

  //return ret;
}
