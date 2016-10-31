#define Database_Foreach(db) { \
Decl_Assertion *assertion = (db)->assertions->read(); \
for (; assertion; assertion = assertion->next->read()) { \
Decl_Expression *expression = assertion->expression->read();


#define Database_Endeach }}
    
