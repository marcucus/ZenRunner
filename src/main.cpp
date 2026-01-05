#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QIcon>

int main(int argc, char *argv[])
{
    // Enable high DPI scaling
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
    
    QGuiApplication app(argc, argv);
    
    // Application metadata
    app.setOrganizationName("ZenRunner");
    app.setOrganizationDomain("zenrunner.dev");
    app.setApplicationName("ZenRunner");
    app.setApplicationVersion("1.0.0");
    
    // Set Qt Quick style for optimal performance
    QQuickStyle::setStyle("Basic");
    
    // Create QML engine
    QQmlApplicationEngine engine;
    
    // Set QML import paths
    engine.addImportPath("qrc:/ui");
    
    // Load main QML file
    const QUrl url(QStringLiteral("qrc:/ui/Main.qml"));
    
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    
    engine.load(url);
    
    return app.exec();
}
