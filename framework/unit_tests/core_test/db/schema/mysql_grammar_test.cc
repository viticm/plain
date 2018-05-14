#include "gtest/gtest.h"
#include "pf/db/connection.h"
#include "pf/db/schema/grammars/mysql_grammar.h"
#include "env.h"
#include "pf/db/schema/blueprint.h"

using namespace pf_db::schema;
using namespace pf_basic::type;

class DBSchemaMysqlGrammar : public testing::Test {

 public:
   static void SetUpTestCase() {
     
     //Normal.
     auto connection = new pf_db::Connection(engine->get_db());
     unique_move(pf_db::Connection, connection, connection_);

     auto mysql_grammar = new grammars::MysqlGrammar;
     unique_move(grammars::Grammar, mysql_grammar, mysql_grammar_);

     auto blueprint = new Blueprint("");
     unique_move(Blueprint, blueprint, blueprint_);
   }

   static void TearDownTestCase() {
     //std::cout << "TearDownTestCase" << std::endl;
   }

 public:

   virtual void SetUp() {
     blueprint_->clear();
   }

   virtual void TearDown() {
   }

 protected:
   static std::unique_ptr<pf_db::Connection> connection_;
   static std::unique_ptr<grammars::Grammar> mysql_grammar_;
   static std::unique_ptr<Blueprint> blueprint_;

};

std::unique_ptr<grammars::Grammar> DBSchemaMysqlGrammar::mysql_grammar_{nullptr};
std::unique_ptr<pf_db::Connection> DBSchemaMysqlGrammar::connection_{nullptr};

void assertEquals(
    const std::vector<std::string> &a, 
    const std::vector<std::string> &b, 
    int32_t line = -1) {
  if (line != -1)
    std::cout << "assertEquals: " << line << std::endl;
  ASSERT_TRUE(a.size() == b.size());
  for (size_t i = 0; i < a.size(); ++i)
    ASSERT_STREQ(a[i].c_str(), b[i].c_str());
}

TEST_F(DBSchemaMysqlGrammar, testBasicCreateTable) {
  blueprint_->set_table("user");
  blueprint_->create();
  blueprint_->increments("id");
  blueprint_->string("email");

}
