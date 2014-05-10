#ifndef curecoinADDRESSVALIDATOR_H
#define curecoinADDRESSVALIDATOR_H

#include <QRegExpValidator>

/** Base48 entry widget validator.
   Corrects near-miss characters and refuses characters that are no part of base48.
 */
class curecoinAddressValidator : public QValidator
{
    Q_OBJECT
public:
    explicit curecoinAddressValidator(QObject *parent = 0);

    State validate(QString &input, int &pos) const;

    static const int MaxAddressLength = 35;
signals:

public slots:

};

#endif // curecoinADDRESSVALIDATOR_H
