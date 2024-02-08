- change to .hpp
- no header in cmaketextlist
- no namespace in header
- no indent in namespace
- namespace == static -> tell linker to not link to it
- static -> compiler
- preprocessor #ifdef.. etc
- static funcs are not declared in headers, due to linker


- does nimble_port_run() hijack main thread?

- default constructors, void* will break it
- const member will break move constructor

- RAII PRIO

- Test if nimble_port_init() cannot be done twice

- fatal_fmt use {} for formating


- make fields a free func
- make flags etc

data vs resource.
no need to have calss that only holds data

- back to basics move klaus