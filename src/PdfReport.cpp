// PdfReport.cpp - writes the simulation report as a real PDF (no external library)
#include "Statistics.h"
#include <cstdio>

namespace Statistics {

static std::string pdfEsc(const std::string& s) {
    std::string r;
    for (size_t i = 0; i < s.size(); i++) {
        char c = s[i];
        if (c == '(' || c == ')' || c == '\\') r += '\\';
        if ((unsigned char)c < 32 || (unsigned char)c > 126) c = '?';
        r += c;
    }
    return r;
}

static void addObj(std::string& out, std::vector<size_t>& off, const std::string& body) {
    char h[32];
    off.push_back(out.size());
    std::snprintf(h, sizeof(h), "%d 0 obj\n", (int)off.size());
    out += h; out += body; out += "\nendobj\n";
}

bool saveReportPdf(const Simulation& sim, const char* path) {
    std::vector<std::string> L = reportLines(sim);
    const int perPage = 50;
    int pages = ((int)L.size() + perPage - 1) / perPage;
    if (pages < 1) pages = 1;

    std::string out = "%PDF-1.4\n";
    std::vector<size_t> off;
    char buf[64];

    std::string kids;
    for (int p = 0; p < pages; p++) { std::snprintf(buf, sizeof(buf), "%d 0 R ", 5 + 2 * p); kids += buf; }
    std::snprintf(buf, sizeof(buf), "%d", pages);

    addObj(out, off, "<< /Type /Catalog /Pages 2 0 R >>");
    addObj(out, off, "<< /Type /Pages /Kids [ " + kids + "] /Count " + std::string(buf) + " >>");
    addObj(out, off, "<< /Type /Font /Subtype /Type1 /BaseFont /Courier >>");
    addObj(out, off, "<< /Type /Font /Subtype /Type1 /BaseFont /Courier-Bold >>");

    for (int p = 0; p < pages; p++) {
        std::string c;
        c += "BT /F2 16 Tf 50 800 Td (CleanCity Simulator - Simulation Report) Tj ET\n";
        c += "1 w 50 790 m 545 790 l S\n";
        for (int i = 0; i < perPage; i++) {
            int idx = p * perPage + i;
            if (idx >= (int)L.size()) break;
            std::snprintf(buf, sizeof(buf), "BT /F1 10 Tf 50 %d Td (", 765 - i * 14);
            c += buf; c += pdfEsc(L[idx]); c += ") Tj ET\n";
        }
        std::snprintf(buf, sizeof(buf), "%d 0 R", 6 + 2 * p);
        addObj(out, off, "<< /Type /Page /Parent 2 0 R /MediaBox [0 0 595 842] /Resources << /Font << /F1 3 0 R /F2 4 0 R >> >> /Contents " + std::string(buf) + " >>");
        std::snprintf(buf, sizeof(buf), "%d", (int)c.size());
        addObj(out, off, "<< /Length " + std::string(buf) + " >>\nstream\n" + c + "endstream");
    }

    size_t xref = out.size();
    std::snprintf(buf, sizeof(buf), "xref\n0 %d\n", (int)off.size() + 1);
    out += buf;
    out += "0000000000 65535 f \n";
    for (size_t i = 0; i < off.size(); i++) { std::snprintf(buf, sizeof(buf), "%010d 00000 n \n", (int)off[i]); out += buf; }
    std::snprintf(buf, sizeof(buf), "trailer\n<< /Size %d /Root 1 0 R >>\nstartxref\n%d\n%%%%EOF\n", (int)off.size() + 1, (int)xref);
    out += buf;

    FILE* f = std::fopen(path, "wb");
    if (!f) return false;
    std::fwrite(out.data(), 1, out.size(), f);
    std::fclose(f);
    return true;
}

} // namespace Statistics
