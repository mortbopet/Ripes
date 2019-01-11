#ifndef CACHESETUPWIDGET_H
#define CACHESETUPWIDGET_H

#include <QWidget>

class CacheBase;

namespace Ui {
class CacheSetupWidget;
}

class CacheSetupWidget : public QWidget {
    Q_OBJECT
    friend class CacheTab;

public:
    explicit CacheSetupWidget(QWidget* parent = nullptr);
    ~CacheSetupWidget() override;

    void setName(QString name);
    void setCachePtr(CacheBase* cachePtr) { m_cachePtr = cachePtr; }

signals:
    void groupBoxToggled(bool state);

public slots:
    void enable(bool state);

private slots:
    void on_groupbox_toggled(bool arg1);

    void cacheSizeChanged(const QString& index);
    void cacheDelayChanged(int);

private:
    Ui::CacheSetupWidget* m_ui;
    CacheBase* m_cachePtr;
};

#endif  // CACHESETUPWIDGET_H
