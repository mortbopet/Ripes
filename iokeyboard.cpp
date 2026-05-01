#pragma once

#include <QLabel>
#include <QMap>
#include <QMutex>
#include <QQueue>
#include <QWidget>

#include "iobase.h"

QT_FORWARD_DECLARE_CLASS(QPushButton);

namespace Ripes {

    class IOKeyboard : public IOBase {
        Q_OBJECT

            enum Parameters { BUFSIZE };

    public:
        IOKeyboard(QWidget* parent);
        ~IOKeyboard() { unregister(); };

        virtual unsigned byteSize() const override { return 8; }
        virtual QString description() const override;
        virtual QString baseName() const override { return "Keyboard"; }

        virtual const std::vector<RegDesc>& registers() const override {
            return m_regDescs;
        };
        virtual const std::vector<IOSymbol>* extraSymbols() const override {
            return &m_extraSymbols;
        }

        virtual VInt ioRead(AInt offset, unsigned size) override;
        virtual void ioWrite(AInt offset, VInt value, unsigned size) override;
        virtual void reset() override;

    protected:
        virtual void parameterChanged(unsigned) override;
        void keyPressEvent(QKeyEvent* event) override;

    private:
        void updateLayout();
        void enqueueKey(uint8_t ascii);
        void refreshStatusLabel();

        QQueue<uint8_t> m_keyBuffer;
        mutable QMutex m_bufMutex;
        uint8_t m_lastKey = 0;

        QLabel* m_statusLabel = nullptr;
        QMap<uint8_t, QPushButton*> m_keys;

        std::vector<RegDesc> m_regDescs;
        std::vector<IOSymbol> m_extraSymbols;
    };

} 