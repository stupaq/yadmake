\section{Słownik pojęć}

Poniżej znajduje się krótki spis pojęć i sformułowań używanych w dalszej części dokumentu wraz z przyjętymi definicjami.

\begin{itemize}
	\item cel, zadanie -- plik będący efektem wykonania określonych poleceń (przepisu) i powiązany z innymi celami zależnościami
	\item zależności -- relacja na celach, mówimy że cel A zależy od B (jest powiązany zależnością) jeśli w procesie kompilacji (budowy kolejnych celów) cel B musi być zbudowany przed A
	\item reguły, przepis -- ciąg poleceń lub wywołań innych programów, prowadzący do wykonania określonego celu przy założeniu, że wszystkie jego zależności (cele, od których zależy) zostały już wykonane
	\item graf kompilacji, graf zależności -- struktura danych dostarczająca informacji o celach, regułach ich budowy oraz łączących je zależnościach
	\item jednostka koordynująca -- komputer, który koordynuje proces kompilacji i na którym został wywołany program do rozproszonej kompilacji
	\item jednostka kompilująca, jednostka pomocnicza -- komputer, który jest gotowy do uczestniczenia w procesie kompilacji, ma zainstalowane i skonfigurowane odpowiednie oprogramowanie oraz jednostka koordynująca może się z nim połączyć i zdalnie wykonywać na nim polecenia oraz synchronizować piki
\end{itemize}

