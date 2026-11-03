#pragma once

#include <QMainWindow>
#include "Viewer.h"
#include "ParamsWidget.h"
#include "model/GeometryFromTopology.h"
#include "logic/GenerateRegularPolytope.h"
#include <memory>
#include <QHBoxLayout>


class MainWindow : public QMainWindow
{
	Q_OBJECT
public:
	explicit MainWindow(QWidget* parent = nullptr);

	Viewer* getViewer() const { return m_viewer; }
	ParamsWidget* getParamsWidget() const { return m_paramsWidget; }

public:
	void onGenerate(int a, int b, int c);
	void onShadedToggled(bool enabled);
private:
	ParamsWidget* m_paramsWidget = nullptr;
	Viewer* m_viewer = nullptr;
	std::unique_ptr<PolytopeGeometry3D> m_currentPolytope;
};
