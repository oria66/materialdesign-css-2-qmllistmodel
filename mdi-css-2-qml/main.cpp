#include <QCoreApplication>

#include <QFile>
#include <QTextStream>
#include <QHashIterator>

QString hex2textstring(int hexCode){

    int h = floor((hexCode - 0x10000) / 0x400) + 0xD800;
    int l = (hexCode - 0x10000) % 0x400 + 0xDC00;

    return QString("\\u%1\\u%2").arg(QString::number(h, 16)).arg(QString::number(l, 16));
}

bool processLineLooking4Key(const QString &line, QString &key){

    // Recognize key line of font with character ::
    if(!line.contains("::"))
        return false;

    QString firstLine = line.split("::").first();

    if(firstLine.isEmpty())
        return false;

    key = firstLine.remove(".mdi-");

    return true;
}

bool processLineLooking4Value(const QString &line, QString &value){

    // Recognize key line of font with character "\\"
    if(!line.contains("\\"))
        return false;

    QString lastLine = line.split("\\").last();

    if(lastLine.isEmpty())
        return false;

    value = "0x" + lastLine.remove("\";");

    return true;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QString originCSSpath("C:/Proyectos/materialdesign-css-2-qmllistmodel/MaterialDesign-Webfont/css/materialdesignicons.css");
    QString finalJSpath("utilsMDI.js");

    QMap<QString, QString> codename;

    QFile file(originCSSpath);

    QTextStream stream(&file);

    if(!file.open(QIODevice::ReadOnly))
        qCritical() << "CSS File cannot be open!" << file.errorString();

    bool isLookingForValue = false;
    QString temporalKey("");
    QString temporalValue("");

    while (!stream.atEnd()) {
        QString line = stream.readLine();

        if(isLookingForValue){
            processLineLooking4Value(line, temporalValue);

            codename.insert(temporalKey.replace("-","_"), temporalValue);

            isLookingForValue = false;
            temporalKey.clear();
        }
        else
            isLookingForValue = processLineLooking4Key(line, temporalKey);
    }

    QFile outputFile(finalJSpath);
    QTextStream out(&outputFile);

    // Can be optimized using QJsonDocument

    if(!outputFile.open(QIODevice::WriteOnly))
        qCritical() << "JS File cannot be open!" << outputFile.errorString();

    out << QByteArray("var mdiIcons = {\n");

    QMapIterator<QString, QString> i(codename);
    while (i.hasNext()) {
        i.next();

        QString finalSeparator(",");

        if(!i.hasNext()){
            finalSeparator.clear();
            finalSeparator = "}";
        }

        out << "\"" << i.key() << "\": "  << i.value() << finalSeparator << "\n";
    }

    outputFile.close();

    QFile finaOutputFile(finalJSpath);
    QTextStream finalStream(&finaOutputFile);

    // Removing \" in key values
    if(!finaOutputFile.open(QIODevice::ReadWrite))
        qCritical() << "JS File cannot be open!" << finaOutputFile.errorString();

    QString alldata = finaOutputFile.readAll().replace("\"","");

    QFile newData(finalJSpath);
    if(newData.open(QFile::WriteOnly | QFile::Truncate)) {
        QTextStream out(&newData);
        out << alldata;
    }
    newData.close();

    return a.exec();
}
