#include "MTPFM.h"
#include <exception>

using namespace System;
using namespace System::Windows;
using namespace System::Windows::Forms;


[STAThreadAttribute]

int main()

{
    Application::EnableVisualStyles();

    Application::SetCompatibleTextRenderingDefault(false);

    MTPFM::MTPFM form;

    Application::Run(% form);

    return 0;
}



