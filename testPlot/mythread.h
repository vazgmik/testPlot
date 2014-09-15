#ifndef MYTHREAD_H
#define MYTHREAD_H

#include <QThread>

class MyThread : public QThread
{
	Q_OBJECT

public:
	MyThread(QObject *parent);
	~MyThread();
	void run();
	bool Stop;
signals:

	
};

#endif // MYTHREAD_H
