#include "SMTSolver.h"
#include <llvm/ADT/APInt.h>
#include <llvm/ADT/SmallString.h>
#include <llvm/Support/Debug.h>
#include <llvm/Support/raw_ostream.h>
#include <assert.h>
#include <stdio.h>
extern "C" {
#include <boolector/boolector.h>
}

#define ctx ((Btor *)ctx_)
#define m   ((Btor *)m_)
#define e   ((BtorExp *)e_)
#define lhs ((BtorExp *)lhs_)
#define rhs ((BtorExp *)rhs_)

SMTSolver::SMTSolver() {
	ctx_ = (SMTContext)boolector_new();
	boolector_enable_model_gen(ctx);
	boolector_enable_inc_usage(ctx);
}

SMTSolver::~SMTSolver() {
	assert(boolector_get_refs(ctx) == 0);
	boolector_delete(ctx);
}

SMTStatus SMTSolver::query(SMTExpr e_, SMTModel *m_) {
	boolector_assume(ctx, e);
	switch (boolector_sat(ctx)) {
	default:              return SMT_UNDEF;
	case BOOLECTOR_UNSAT: return SMT_UNSAT;
	case BOOLECTOR_SAT:   break;
	}
	if (m_)
		*m_ = ctx_;
	return SMT_SAT;
}

void SMTSolver::eval(SMTModel m_, SMTExpr e_, llvm::raw_ostream &OS) {
	char *s = boolector_bv_assignment(ctx, e);
	OS << s;
	boolector_free_bv_assignment(ctx, s);
}

void SMTSolver::release(SMTModel m_) {}

void SMTSolver::dump(SMTExpr e_) {
	print(e, llvm::dbgs());
	llvm::dbgs() << "\n";
}

void SMTSolver::print(SMTExpr e_, llvm::raw_ostream &OS) {
	FILE *fp = tmpfile();
	assert(fp && "tmpfile");
	boolector_dump_smt(ctx, fp, e);
	long n = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	char *s = (char *)malloc(n + 1);
	n = fread(s, 1, n, fp);
	assert(n > 0);
	s[n] = 0;
	OS << s;
	free(s);
	fclose(fp);
}

void SMTSolver::incref(SMTExpr e_) {
	boolector_copy(ctx, e);
}

void SMTSolver::decref(SMTExpr e_) {
	boolector_release(ctx, e);
}

unsigned SMTSolver::bvwidth(SMTExpr e_) {
	return boolector_get_width(ctx, e);
}

SMTExpr SMTSolver::bvfalse() {
	return boolector_false(ctx);
}

SMTExpr SMTSolver::bvtrue() {
	return boolector_true(ctx);
}

SMTExpr SMTSolver::bvconst(const llvm::APInt &Val) {
	unsigned intbits = sizeof(unsigned) * CHAR_BIT;
	unsigned width = Val.getBitWidth();
	if (width <= intbits)
		return boolector_unsigned_int(ctx, Val.getZExtValue(), width);
	llvm::SmallString<32> Str, FullStr;
	Val.toStringUnsigned(Str, 2);
	assert(Str.size() <= width);
	FullStr.assign(width - Str.size(), '0');
	FullStr += Str;
	return boolector_const(ctx, FullStr.c_str());
}

SMTExpr SMTSolver::bvvar(unsigned width, const char *name) {
	return boolector_var(ctx, width, name);
}

SMTExpr SMTSolver::ite(SMTExpr e_, SMTExpr lhs_, SMTExpr rhs_) {
	return boolector_cond(ctx, e, lhs, rhs);
}

SMTExpr SMTSolver::eq(SMTExpr lhs_, SMTExpr rhs_) {
	return boolector_eq(ctx, lhs, rhs);
}

SMTExpr SMTSolver::ne(SMTExpr lhs_, SMTExpr rhs_) {
	return boolector_ne(ctx, lhs, rhs);
}

SMTExpr SMTSolver::bvslt(SMTExpr lhs_, SMTExpr rhs_) {
	return boolector_slt(ctx, lhs, rhs);
}

SMTExpr SMTSolver::bvsle(SMTExpr lhs_, SMTExpr rhs_) {
	return boolector_slte(ctx, lhs, rhs);
}

SMTExpr SMTSolver::bvsgt(SMTExpr lhs_, SMTExpr rhs_) {
	return boolector_sgt(ctx, lhs, rhs);
}

SMTExpr SMTSolver::bvsge(SMTExpr lhs_, SMTExpr rhs_) {
	return boolector_sgte(ctx, lhs, rhs);
}

SMTExpr SMTSolver::bvult(SMTExpr lhs_, SMTExpr rhs_) {
	return boolector_ult(ctx, lhs, rhs);
}

SMTExpr SMTSolver::bvule(SMTExpr lhs_, SMTExpr rhs_) {
	return boolector_ulte(ctx, lhs, rhs);
}

SMTExpr SMTSolver::bvugt(SMTExpr lhs_, SMTExpr rhs_) {
	return boolector_ugt(ctx, lhs, rhs);
}

SMTExpr SMTSolver::bvuge(SMTExpr lhs_, SMTExpr rhs_) {
	return boolector_ugte(ctx, lhs, rhs);
}

SMTExpr SMTSolver::extract(unsigned high, unsigned low, SMTExpr e_) {
	return boolector_slice(ctx, e, high, low);
}

SMTExpr SMTSolver::zero_extend(unsigned i, SMTExpr e_) {
	return boolector_uext(ctx, e, i);
}

SMTExpr SMTSolver::sign_extend(unsigned i, SMTExpr e_) {
	return boolector_sext(ctx, e, i);
}

SMTExpr SMTSolver::bvredand(SMTExpr e_) {
	return boolector_redand(ctx, e);
}

SMTExpr SMTSolver::bvredor(SMTExpr e_) {
	return boolector_redor(ctx, e);
}

SMTExpr SMTSolver::bvnot(SMTExpr e_) {
	return boolector_not(ctx, e);
}

SMTExpr SMTSolver::bvneg(SMTExpr e_) {
	return boolector_neg(ctx, e);
}

SMTExpr SMTSolver::bvadd(SMTExpr lhs_, SMTExpr rhs_) {
	return boolector_add(ctx, lhs, rhs);
}

SMTExpr SMTSolver::bvsub(SMTExpr lhs_, SMTExpr rhs_) {
	return boolector_sub(ctx, lhs, rhs);
}

SMTExpr SMTSolver::bvmul(SMTExpr lhs_, SMTExpr rhs_) {
	return boolector_mul(ctx, lhs, rhs);
}

SMTExpr SMTSolver::bvsdiv(SMTExpr lhs_, SMTExpr rhs_) {
	return boolector_sdiv(ctx, lhs, rhs);
}

SMTExpr SMTSolver::bvudiv(SMTExpr lhs_, SMTExpr rhs_) {
	return boolector_udiv(ctx, lhs, rhs);
}

SMTExpr SMTSolver::bvsrem(SMTExpr lhs_, SMTExpr rhs_) {
	return boolector_srem(ctx, lhs, rhs);
}

SMTExpr SMTSolver::bvurem(SMTExpr lhs_, SMTExpr rhs_) {
	return boolector_urem(ctx, lhs, rhs);
}

// Shift operations use log2n bits for shifting amount. 
template <BtorExp *(*F)(Btor *, BtorExp *,  BtorExp *)>
static inline SMTExpr shift(Btor *btor,  BtorExp *e0,  BtorExp *e1) {
	unsigned n = boolector_get_width(btor, e1);
	assert(!(n & (n - 1)) && "TODO");
	unsigned amount = __builtin_ctz(n);
	BtorExp *log2w = boolector_slice(btor, e1, amount - 1, 0);
	SMTExpr tmp = F(btor, e0, log2w);
	boolector_release(btor, log2w);
	return tmp;
}

SMTExpr SMTSolver::bvshl(SMTExpr lhs_, SMTExpr rhs_) {
	return shift<boolector_sll>(ctx, lhs, rhs);
}

SMTExpr SMTSolver::bvlshr(SMTExpr lhs_, SMTExpr rhs_) {
	return shift<boolector_srl>(ctx, lhs, rhs);
}

SMTExpr SMTSolver::bvashr(SMTExpr lhs_, SMTExpr rhs_) {
	return shift<boolector_sra>(ctx, lhs, rhs);
}

SMTExpr SMTSolver::bvand(SMTExpr lhs_, SMTExpr rhs_) {
	return boolector_and(ctx, lhs, rhs);
}

SMTExpr SMTSolver::bvor(SMTExpr lhs_, SMTExpr rhs_) {
	return boolector_or(ctx, lhs, rhs);
}

SMTExpr SMTSolver::bvxor(SMTExpr lhs_, SMTExpr rhs_) {
	return boolector_xor(ctx, lhs, rhs);
}

SMTExpr SMTSolver::bvneg_overflow(SMTExpr e_) {
	SMTExpr zero = boolector_zero(ctx, bvwidth(e));
	SMTExpr tmp = bvssub_overflow(zero, e);
	decref(zero);
	return tmp;
}

SMTExpr SMTSolver::bvsadd_overflow(SMTExpr lhs_, SMTExpr rhs_) {
	return boolector_saddo(ctx, lhs, rhs);
}

SMTExpr SMTSolver::bvuadd_overflow(SMTExpr lhs_, SMTExpr rhs_) {
	return boolector_uaddo(ctx, lhs, rhs);
}

SMTExpr SMTSolver::bvssub_overflow(SMTExpr lhs_, SMTExpr rhs_) {
	return boolector_ssubo(ctx, lhs, rhs);
}

SMTExpr SMTSolver::bvusub_overflow(SMTExpr lhs_, SMTExpr rhs_) {
	return boolector_usubo(ctx, lhs, rhs);
}

SMTExpr SMTSolver::bvsmul_overflow(SMTExpr lhs_, SMTExpr rhs_) {
	return boolector_smulo(ctx, lhs, rhs);
}

SMTExpr SMTSolver::bvumul_overflow(SMTExpr lhs_, SMTExpr rhs_) {
	return boolector_umulo(ctx, lhs, rhs);
}

SMTExpr SMTSolver::bvsdiv_overflow(SMTExpr lhs_, SMTExpr rhs_) {
	return boolector_sdivo(ctx, lhs, rhs);
}